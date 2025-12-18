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

#include "BridgeTool.h"
#include "BridgeToolPage.h"

#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Selection.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"

#include <algorithm>
#include <cmath>

namespace tb::ui
{

BridgeTool::BridgeTool(MapDocument& document)
  : Tool{false}
  , m_document{document}
{
}

bool BridgeTool::doActivate()
{
  return true;
}

void BridgeTool::setSegments(int segments)
{
  m_segments = std::max(1, segments);
}

int BridgeTool::segments() const
{
  return m_segments;
}

void BridgeTool::setCurved(bool curved)
{
  m_curvedBridge = curved;
}

bool BridgeTool::isCurved() const
{
  return m_curvedBridge;
}

void BridgeTool::setCurvature(double curvature)
{
  m_curvature = std::clamp(curvature, -1.0, 1.0);
}

double BridgeTool::curvature() const
{
  return m_curvature;
}

void BridgeTool::setTaper(bool taper)
{
  m_taper = taper;
}

bool BridgeTool::isTapered() const
{
  return m_taper;
}

void BridgeTool::setTaperAmount(double amount)
{
  m_taperAmount = std::clamp(amount, -1.0, 1.0);
}

double BridgeTool::taperAmount() const
{
  return m_taperAmount;
}

std::optional<std::pair<mdl::BrushFaceHandle, mdl::BrushFaceHandle>> BridgeTool::getSelectedFaces() const
{
  const auto& map = m_document.map();
  const auto& selection = map.selection();
  
  if (selection.brushFaces.size() != 2)
  {
    return std::nullopt;
  }
  
  return std::make_pair(selection.brushFaces[0], selection.brushFaces[1]);
}

bool BridgeTool::canCreateBridge() const
{
  return getSelectedFaces().has_value();
}

bool BridgeTool::createBridge()
{
  if (!canCreateBridge())
  {
    return false;
  }

  auto selectedFaces = getSelectedFaces();
  if (!selectedFaces)
  {
    return false;
  }

  auto& map = m_document.map();
  auto transaction = mdl::Transaction{map, "Create Bridge"};

  const auto& face1 = selectedFaces->first.face();
  const auto& face2 = selectedFaces->second.face();

  // Get face centers and normals
  const auto center1 = face1.boundsCenter();
  const auto center2 = face2.boundsCenter();
  const auto normal1 = face1.normal();
  const auto normal2 = face2.normal();

  // Calculate base vectors for each face
  const auto& vertices1 = face1.vertices();
  const auto& vertices2 = face2.vertices();

  if (vertices1.size() < 3 || vertices2.size() < 3)
  {
    return false;
  }

  // Get the bounding boxes of the faces for sizing
  auto builder1 = vm::bbox3d::builder{};
  for (const auto* v : vertices1)
  {
    builder1.add(v->position());
  }
  const auto bounds1 = builder1.bounds();
  
  auto builder2 = vm::bbox3d::builder{};
  for (const auto* v : vertices2)
  {
    builder2.add(v->position());
  }
  const auto bounds2 = builder2.bounds();

  const auto size1 = bounds1.size();
  const auto size2 = bounds2.size();
  
  // Average size for the bridge cross-section
  double avgWidth = (std::max(size1.x(), size1.y()) + std::max(size2.x(), size2.y())) / 2.0;
  double avgHeight = (size1.z() + size2.z()) / 2.0;
  if (avgHeight < 1.0) avgHeight = avgWidth;
  if (avgWidth < 1.0) avgWidth = 8.0;

  // Create bridge segments
  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};

  std::vector<mdl::Node*> newNodes;

  for (int i = 0; i < m_segments; ++i)
  {
    const double t1 = static_cast<double>(i) / static_cast<double>(m_segments);
    const double t2 = static_cast<double>(i + 1) / static_cast<double>(m_segments);

    // Interpolate positions
    auto pos1 = center1 + (center2 - center1) * t1;
    auto pos2 = center1 + (center2 - center1) * t2;

    // Apply curvature if enabled
    if (m_curvedBridge && std::abs(m_curvature) > 0.001)
    {
      // Simple parabolic curve
      const double curveOffset1 = m_curvature * 4.0 * t1 * (1.0 - t1) * avgWidth;
      const double curveOffset2 = m_curvature * 4.0 * t2 * (1.0 - t2) * avgWidth;
      
      // Apply curve perpendicular to the bridge direction
      const auto bridgeDir = vm::normalize(center2 - center1);
      const auto upDir = vm::vec3d{0, 0, 1};
      const auto sideDir = vm::normalize(vm::cross(bridgeDir, upDir));
      
      pos1 = pos1 + sideDir * curveOffset1;
      pos2 = pos2 + sideDir * curveOffset2;
    }

    // Apply taper if enabled
    double scale1 = 1.0;
    double scale2 = 1.0;
    if (m_taper && std::abs(m_taperAmount) > 0.001)
    {
      // Taper towards the middle
      const double taperMid = 1.0 - std::abs(m_taperAmount);
      scale1 = 1.0 - (1.0 - taperMid) * 2.0 * std::abs(t1 - 0.5);
      scale2 = 1.0 - (1.0 - taperMid) * 2.0 * std::abs(t2 - 0.5);
    }

    // Create a cuboid for this segment
    const double segmentWidth1 = avgWidth * scale1;
    const double segmentWidth2 = avgWidth * scale2;
    const double segmentHeight = avgHeight;

    // Use the midpoint for the segment
    const auto segmentCenter = (pos1 + pos2) * 0.5;
    const auto segmentLength = vm::length(pos2 - pos1);

    if (segmentLength < 0.1) continue;

    // Create segment bounds
    const auto halfSize = vm::vec3d{segmentLength / 2.0 + 0.1, (segmentWidth1 + segmentWidth2) / 4.0, segmentHeight / 2.0};
    const auto segmentBounds = vm::bbox3d{segmentCenter - halfSize, segmentCenter + halfSize};

    auto brushResult = builder.createCuboid(segmentBounds, map.currentMaterialName());
    if (brushResult.is_success())
    {
      auto* brushNode = new mdl::BrushNode{std::move(brushResult.value())};
      newNodes.push_back(brushNode);
    }
  }

  if (newNodes.empty())
  {
    transaction.cancel();
    return false;
  }

  // Add nodes to the map
  auto* parent = mdl::parentForNodes(map);
  if (mdl::addNodes(map, {{parent, newNodes}}).empty())
  {
    transaction.cancel();
    return false;
  }

  // Select the new nodes
  mdl::deselectAll(map);
  mdl::selectNodes(map, newNodes);

  transaction.commit();
  return true;
}

QWidget* BridgeTool::doCreatePage(QWidget* parent)
{
  return new BridgeToolPage{m_document, *this, parent};
}

} // namespace tb::ui
