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

#include "SmartSnap.h"

#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Node.h"

#include "kd/overload.h"

#include "vm/distance.h"
#include "vm/scalar.h"

#include <cmath>
#include <limits>

namespace tb::ui
{

std::optional<SmartSnap::SnapResult> SmartSnap::findSnapTarget(
  const vm::vec3d& position,
  const double gridSize,
  const SmartSnapMode mode,
  const std::vector<const mdl::Node*>& candidates,
  const double snapThreshold)
{
  if (mode == SmartSnapMode::GridOnly)
  {
    return SnapResult{
      snapToGrid(position, gridSize),
      SmartSnapMode::GridOnly,
      std::nullopt,
      0.0
    };
  }

  std::optional<SnapResult> bestResult;
  double bestDistance = std::numeric_limits<double>::max();

  // Try vertex snap
  if (mode == SmartSnapMode::All || mode == SmartSnapMode::Vertices)
  {
    if (auto result = snapToVertex(position, candidates, snapThreshold))
    {
      if (result->distance < bestDistance)
      {
        bestDistance = result->distance;
        bestResult = result;
      }
    }
  }

  // Try edge snap
  if (mode == SmartSnapMode::All || mode == SmartSnapMode::Edges)
  {
    if (auto result = snapToEdge(position, candidates, snapThreshold))
    {
      if (result->distance < bestDistance)
      {
        bestDistance = result->distance;
        bestResult = result;
      }
    }
  }

  // Try center snap
  if (mode == SmartSnapMode::All || mode == SmartSnapMode::ObjectCenters)
  {
    if (auto result = snapToCenter(position, candidates, snapThreshold))
    {
      if (result->distance < bestDistance)
      {
        bestDistance = result->distance;
        bestResult = result;
      }
    }
  }

  // Fall back to grid snap if no geometry snap found
  if (!bestResult)
  {
    return SnapResult{
      snapToGrid(position, gridSize),
      SmartSnapMode::GridOnly,
      std::nullopt,
      0.0
    };
  }

  return bestResult;
}

vm::vec3d SmartSnap::snapToGrid(const vm::vec3d& position, const double gridSize)
{
  if (gridSize <= 0.0)
  {
    return position;
  }
  
  return vm::vec3d{
    std::round(position.x() / gridSize) * gridSize,
    std::round(position.y() / gridSize) * gridSize,
    std::round(position.z() / gridSize) * gridSize
  };
}

std::optional<SmartSnap::SnapResult> SmartSnap::snapToVertex(
  const vm::vec3d& position,
  const std::vector<const mdl::Node*>& candidates,
  const double threshold)
{
  std::optional<SnapResult> best;
  double bestDist = threshold;

  for (const auto* node : candidates)
  {
    node->accept(kdl::overload(
      [](const mdl::WorldNode*) {},
      [](const mdl::LayerNode*) {},
      [&](const mdl::GroupNode* groupNode) {
        const auto center = groupNode->logicalBounds().center();
        const auto dist = vm::length(position - center);
        if (dist < bestDist)
        {
          bestDist = dist;
          best = SnapResult{center, SmartSnapMode::Vertices, groupNode, dist};
        }
      },
      [&](const mdl::EntityNode* entityNode) {
        const auto center = entityNode->logicalBounds().center();
        const auto dist = vm::length(position - center);
        if (dist < bestDist)
        {
          bestDist = dist;
          best = SnapResult{center, SmartSnapMode::Vertices, entityNode, dist};
        }
      },
      [&](const mdl::BrushNode* brushNode) {
        const auto& brush = brushNode->brush();
        for (const auto* vertex : brush.vertices())
        {
          const auto dist = vm::length(position - vertex->position());
          if (dist < bestDist)
          {
            bestDist = dist;
            best = SnapResult{vertex->position(), SmartSnapMode::Vertices, brushNode, dist};
          }
        }
      },
      [](const mdl::PatchNode*) {}
    ));
  }

  return best;
}

std::optional<SmartSnap::SnapResult> SmartSnap::snapToEdge(
  const vm::vec3d& position,
  const std::vector<const mdl::Node*>& candidates,
  const double threshold)
{
  std::optional<SnapResult> best;
  double bestDist = threshold;

  for (const auto* node : candidates)
  {
    node->accept(kdl::overload(
      [](const mdl::WorldNode*) {},
      [](const mdl::LayerNode*) {},
      [](const mdl::GroupNode*) {},
      [](const mdl::EntityNode*) {},
      [&](const mdl::BrushNode* brushNode) {
        const auto& brush = brushNode->brush();
        for (const auto* edge : brush.edges())
        {
          const auto p1 = edge->firstVertex()->position();
          const auto p2 = edge->secondVertex()->position();
          
          // Find closest point on edge
          const auto edgeVec = p2 - p1;
          const auto edgeLen = vm::length(edgeVec);
          if (edgeLen < 0.001) continue;
          
          const auto edgeDir = edgeVec / edgeLen;
          const auto t = std::clamp(vm::dot(position - p1, edgeDir), 0.0, edgeLen);
          const auto closestPoint = p1 + edgeDir * t;
          
          const auto dist = vm::length(position - closestPoint);
          if (dist < bestDist)
          {
            bestDist = dist;
            best = SnapResult{closestPoint, SmartSnapMode::Edges, brushNode, dist};
          }
        }
      },
      [](const mdl::PatchNode*) {}
    ));
  }

  return best;
}

std::optional<SmartSnap::SnapResult> SmartSnap::snapToCenter(
  const vm::vec3d& position,
  const std::vector<const mdl::Node*>& candidates,
  const double threshold)
{
  std::optional<SnapResult> best;
  double bestDist = threshold;

  for (const auto* node : candidates)
  {
    const auto center = node->logicalBounds().center();
    const auto dist = vm::length(position - center);
    if (dist < bestDist)
    {
      bestDist = dist;
      best = SnapResult{center, SmartSnapMode::ObjectCenters, node, dist};
    }
  }

  return best;
}

} // namespace tb::ui
