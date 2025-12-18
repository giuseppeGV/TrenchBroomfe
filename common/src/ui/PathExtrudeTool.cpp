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

#include "PathExtrudeTool.h"
#include "PathExtrudeToolPage.h"

#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
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
#include "vm/quat.h"

#include <algorithm>
#include <cmath>

namespace tb::ui
{

PathExtrudeTool::PathExtrudeTool(MapDocument& document)
  : Tool{false}
  , m_document{document}
{
}

bool PathExtrudeTool::doActivate()
{
  return true;
}

bool PathExtrudeTool::doDeactivate()
{
  clearWaypoints();
  return true;
}

void PathExtrudeTool::addWaypoint(const vm::vec3d& point)
{
  m_waypoints.push_back(point);
}

void PathExtrudeTool::removeLastWaypoint()
{
  if (!m_waypoints.empty())
  {
    m_waypoints.pop_back();
  }
}

void PathExtrudeTool::clearWaypoints()
{
  m_waypoints.clear();
}

size_t PathExtrudeTool::waypointCount() const
{
  return m_waypoints.size();
}

const std::vector<vm::vec3d>& PathExtrudeTool::waypoints() const
{
  return m_waypoints;
}

void PathExtrudeTool::setSegments(int segments)
{
  m_segments = std::max(1, segments);
}

int PathExtrudeTool::segments() const
{
  return m_segments;
}

void PathExtrudeTool::setAlignToPath(bool align)
{
  m_alignToPath = align;
}

bool PathExtrudeTool::alignToPath() const
{
  return m_alignToPath;
}

void PathExtrudeTool::setScaleAlongPath(bool scale)
{
  m_scaleAlongPath = scale;
}

bool PathExtrudeTool::scaleAlongPath() const
{
  return m_scaleAlongPath;
}

void PathExtrudeTool::setStartScale(double scale)
{
  m_startScale = std::max(0.01, scale);
}

double PathExtrudeTool::startScale() const
{
  return m_startScale;
}

void PathExtrudeTool::setEndScale(double scale)
{
  m_endScale = std::max(0.01, scale);
}

double PathExtrudeTool::endScale() const
{
  return m_endScale;
}

void PathExtrudeTool::setTwist(bool twist)
{
  m_twist = twist;
}

bool PathExtrudeTool::twist() const
{
  return m_twist;
}

void PathExtrudeTool::setTwistAngle(double angle)
{
  m_twistAngle = angle;
}

double PathExtrudeTool::twistAngle() const
{
  return m_twistAngle;
}

bool PathExtrudeTool::canExtrude() const
{
  if (m_waypoints.size() < 2)
  {
    return false;
  }
  
  const auto& map = m_document.map();
  const auto& selection = map.selection();
  return selection.hasOnlyBrushes() && !selection.brushes.empty();
}

bool PathExtrudeTool::performExtrusion()
{
  if (!canExtrude())
  {
    return false;
  }

  auto& map = m_document.map();
  auto transaction = mdl::Transaction{map, "Path Extrude"};

  const auto& selection = map.selection();
  const auto sourceBrushes = selection.brushes;
  
  if (sourceBrushes.empty())
  {
    return false;
  }

  // Calculate source center
  auto sourceCenter = vm::vec3d{0, 0, 0};
  for (const auto* brushNode : sourceBrushes)
  {
    sourceCenter = sourceCenter + brushNode->logicalBounds().center();
  }
  sourceCenter = sourceCenter / static_cast<double>(sourceBrushes.size());

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};

  std::vector<mdl::Node*> newNodes;

  // Interpolate along path
  const size_t totalSegments = (m_waypoints.size() - 1) * static_cast<size_t>(m_segments);
  
  for (size_t i = 0; i <= totalSegments; ++i)
  {
    const double t = static_cast<double>(i) / static_cast<double>(totalSegments);
    
    // Find position along path using linear interpolation
    const double pathT = t * static_cast<double>(m_waypoints.size() - 1);
    const size_t idx = static_cast<size_t>(pathT);
    const double localT = pathT - static_cast<double>(idx);
    
    const size_t nextIdx = std::min(idx + 1, m_waypoints.size() - 1);
    const auto position = m_waypoints[idx] + (m_waypoints[nextIdx] - m_waypoints[idx]) * localT;

    // Calculate path direction for alignment
    auto direction = vm::vec3d{0, 1, 0};
    if (m_alignToPath && idx < m_waypoints.size() - 1)
    {
      direction = vm::normalize(m_waypoints[nextIdx] - m_waypoints[idx]);
    }

    // Calculate scale
    double scale = 1.0;
    if (m_scaleAlongPath)
    {
      scale = m_startScale + (m_endScale - m_startScale) * t;
    }

    // Calculate twist rotation
    double twistRad = 0.0;
    if (m_twist)
    {
      twistRad = vm::to_radians(m_twistAngle * static_cast<double>(i));
    }

    // Clone and transform each source brush
    for (const auto* sourceBrushNode : sourceBrushes)
    {
      auto brush = sourceBrushNode->brush();
      
      // Build transformation matrix
      // 1. Move to origin
      auto transform = vm::translation_matrix(-sourceCenter);
      
      // 2. Apply scaling
      if (m_scaleAlongPath && std::abs(scale - 1.0) > 0.001)
      {
        transform = vm::scaling_matrix(vm::vec3d{scale, scale, scale}) * transform;
      }
      
      // 3. Apply twist
      if (m_twist && std::abs(twistRad) > 0.001)
      {
        transform = vm::rotation_matrix(vm::vec3d{0, 0, 1}, twistRad) * transform;
      }
      
      // 4. Align to path direction
      if (m_alignToPath)
      {
        // Calculate rotation to align Y axis with path direction
        const auto up = vm::vec3d{0, 0, 1};
        const auto right = vm::normalize(vm::cross(up, direction));
        const auto newUp = vm::normalize(vm::cross(direction, right));
        
        // Build rotation matrix from axes
        auto rotMat = vm::mat4x4d::identity();
        rotMat[0][0] = right.x(); rotMat[0][1] = direction.x(); rotMat[0][2] = newUp.x();
        rotMat[1][0] = right.y(); rotMat[1][1] = direction.y(); rotMat[1][2] = newUp.y();
        rotMat[2][0] = right.z(); rotMat[2][1] = direction.z(); rotMat[2][2] = newUp.z();
        
        transform = rotMat * transform;
      }
      
      // 5. Move to path position
      transform = vm::translation_matrix(position) * transform;
      
      // Apply transformation
      auto result = brush.transform(map.worldBounds(), transform, false);
      if (result.is_success())
      {
        auto* brushNode = new mdl::BrushNode{std::move(brush)};
        newNodes.push_back(brushNode);
      }
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
  clearWaypoints();
  return true;
}

QWidget* PathExtrudeTool::doCreatePage(QWidget* parent)
{
  return new PathExtrudeToolPage{m_document, *this, parent};
}

} // namespace tb::ui
