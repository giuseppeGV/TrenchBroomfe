/*
 Copyright (C) 2024 TrenchBroom Contributors

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MeasureTool.h"
#include "MeasureToolPage.h"

#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"
#include "mdl/Selection.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"

#include "vm/distance.h"
#include "vm/polygon.h"

#include "kd/overload.h"

#include <cmath>

namespace tb::ui
{

MeasureTool::MeasureTool(MapDocument& document)
  : Tool{false}
  , m_document{document}
{
}

bool MeasureTool::doActivate()
{
  return true;
}

const mdl::Grid& MeasureTool::grid() const
{
  return m_document.map().grid();
}

void MeasureTool::setStartPoint(const vm::vec3d& point)
{
  m_measureStartPoint = point;
}

void MeasureTool::setEndPoint(const vm::vec3d& point)
{
  m_measureEndPoint = point;
}

void MeasureTool::clearMeasurement()
{
  m_measureStartPoint = std::nullopt;
  m_measureEndPoint = std::nullopt;
}

std::optional<vm::vec3d> MeasureTool::startPoint() const
{
  return m_measureStartPoint;
}

std::optional<vm::vec3d> MeasureTool::endPoint() const
{
  return m_measureEndPoint;
}

double MeasureTool::calculateDistance() const
{
  if (m_measureStartPoint && m_measureEndPoint)
  {
    return vm::length(*m_measureEndPoint - *m_measureStartPoint);
  }
  return 0.0;
}

vm::vec3d MeasureTool::calculateComponentDistances() const
{
  if (m_measureStartPoint && m_measureEndPoint)
  {
    const auto delta = *m_measureEndPoint - *m_measureStartPoint;
    return vm::vec3d{std::abs(delta.x()), std::abs(delta.y()), std::abs(delta.z())};
  }
  return vm::vec3d{0, 0, 0};
}

MeasurementResult MeasureTool::measureSelection() const
{
  MeasurementResult result;
  
  const auto& map = m_document.map();
  if (!map.selection().hasNodes())
  {
    return result;
  }

  const auto bounds = *map.selectionBounds();
  const auto size = bounds.size();

  result.distanceX = size.x();
  result.distanceY = size.y();
  result.distanceZ = size.z();
  result.totalDistance = vm::length(size);

  // Count nodes and calculate properties
  for (auto* node : map.selection().nodes)
  {
    node->accept(kdl::overload(
      [&](mdl::BrushNode* brushNode) {
        result.brushCount++;
        const auto& brush = brushNode->brush();
        result.faceCount += static_cast<int>(brush.faceCount());
        result.vertexCount += static_cast<int>(brush.vertexCount());
        
        // Calculate brush volume (approximation using bounding box)
        const auto brushBounds = brushNode->logicalBounds();
        const auto brushSize = brushBounds.size();
        result.volume += brushSize.x() * brushSize.y() * brushSize.z();
        
        // Calculate face areas
        for (size_t i = 0; i < brush.faceCount(); ++i)
        {
          const auto& face = brush.face(i);
          // Calculate face area from vertices
          const auto& vertices = face.vertices();
          if (vertices.size() >= 3)
          {
            // Use the shoelace formula for polygon area in 3D
            // We project onto the dominant plane based on the face normal
            // Simple area calculation using cross products
            double faceArea = 0.0;
            // Vertices are pointers, so store them as such
            const auto verticesVec = std::vector<const mdl::BrushVertex*>(vertices.begin(), vertices.end());
            const auto& v0 = verticesVec[0]->position();
            for (size_t j = 1; j < verticesVec.size() - 1; ++j)
            {
              const auto& v1 = verticesVec[j]->position();
              const auto& v2 = verticesVec[j + 1]->position();
              const auto cross = vm::cross(v1 - v0, v2 - v0);
              faceArea += vm::length(cross) * 0.5;
            }
            result.area += faceArea;
          }
        }
      },
      [&](mdl::EntityNode*) {
        result.entityCount++;
      },
      [&](mdl::GroupNode*) {
        // Groups are containers, their children are counted separately
      },
      [&](mdl::PatchNode*) {
        // Patches could have their own area calculation
      },
      [](mdl::WorldNode*) {},
      [](mdl::LayerNode*) {}
    ));
  }

  return result;
}

vm::vec3d MeasureTool::selectionDimensions() const
{
  const auto& map = m_document.map();
  if (!map.selection().hasNodes())
  {
    return vm::vec3d{0, 0, 0};
  }

  const auto bounds = *map.selectionBounds();
  return bounds.size();
}

double MeasureTool::calculateTotalFaceArea() const
{
  const auto result = measureSelection();
  return result.area;
}

double MeasureTool::calculateTotalVolume() const
{
  const auto result = measureSelection();
  return result.volume;
}

QWidget* MeasureTool::doCreatePage(QWidget* parent)
{
  return new MeasureToolPage{m_document, *this, parent};
}

} // namespace tb::ui
