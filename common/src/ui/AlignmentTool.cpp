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

#include "AlignmentTool.h"
#include "AlignmentToolPage.h"

#include "mdl/Brush.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/NodeContents.h"
#include "mdl/PatchNode.h"
#include "mdl/Selection.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"

#include "kd/overload.h"

#include <algorithm>
#include <vector>

namespace tb::ui
{

AlignmentTool::AlignmentTool(MapDocument& document)
  : Tool{false}
  , m_document{document}
{
}

bool AlignmentTool::doActivate()
{
  return true;
}

const mdl::Grid& AlignmentTool::grid() const
{
  return m_document.map().grid();
}

void AlignmentTool::alignObjects(int axis, AlignMode mode, bool alignToFirst)
{
  auto& map = m_document.map();
  
  if (!map.selection().hasNodes())
  {
    return;
  }

  using namespace tb::mdl;

  auto nodes = map.selection().nodes;
  if (nodes.size() < 2 && !alignToFirst)
  {
    return;
  }

  auto transaction = Transaction{map, "Align Objects"};

  // Calculate target position based on mode
  double targetPos = 0.0;

  if (alignToFirst && !nodes.empty())
  {
    // Use first selected object as reference
    const auto firstBounds = nodes[0]->logicalBounds();
    switch (mode)
    {
    case AlignMode::Min:
      targetPos = firstBounds.min[static_cast<size_t>(axis)];
      break;
    case AlignMode::Center:
      targetPos = firstBounds.center()[static_cast<size_t>(axis)];
      break;
    case AlignMode::Max:
      targetPos = firstBounds.max[static_cast<size_t>(axis)];
      break;
    }
  }
  else
  {
    // Calculate bounds of all selected objects
    const auto totalBounds = map.selection().selectionBounds();
    switch (mode)
    {
    case AlignMode::Min:
      targetPos = totalBounds.min[static_cast<size_t>(axis)];
      break;
    case AlignMode::Center:
      targetPos = totalBounds.center()[static_cast<size_t>(axis)];
      break;
    case AlignMode::Max:
      targetPos = totalBounds.max[static_cast<size_t>(axis)];
      break;
    }
  }

  // Apply alignment to each object (skip first if alignToFirst)
  for (size_t i = (alignToFirst ? 1 : 0); i < nodes.size(); ++i)
  {
    auto* node = nodes[i];
    const auto bounds = node->logicalBounds();
    
    double currentPos = 0.0;
    switch (mode)
    {
    case AlignMode::Min:
      currentPos = bounds.min[static_cast<size_t>(axis)];
      break;
    case AlignMode::Center:
      currentPos = bounds.center()[static_cast<size_t>(axis)];
      break;
    case AlignMode::Max:
      currentPos = bounds.max[static_cast<size_t>(axis)];
      break;
    }

    const double delta = targetPos - currentPos;
    if (std::abs(delta) > 0.001)
    {
      vm::vec3d translation{0, 0, 0};
      translation[static_cast<size_t>(axis)] = delta;
      
      const auto transform = vm::translation_matrix(translation);
      
      node->accept(kdl::overload(
        [&](BrushNode* brushNode) {
          auto brush = brushNode->brush();
          brush.transform(map.worldBounds(), transform, false);
          updateNodeContents(map, "Align", {{brushNode, NodeContents{std::move(brush)}}}, {});
        },
        [&](EntityNode* entityNode) {
          auto entity = entityNode->entity();
          entity.transform(transform, false);
          updateNodeContents(map, "Align", {{entityNode, NodeContents{std::move(entity)}}}, {});
        },
        [&](GroupNode* groupNode) {
          auto group = groupNode->group();
          group.transform(transform);
          updateNodeContents(map, "Align", {{groupNode, NodeContents{std::move(group)}}}, {});
        },
        [&](PatchNode* patchNode) {
          auto patch = patchNode->patch();
          patch.transform(transform);
          updateNodeContents(map, "Align", {{patchNode, NodeContents{std::move(patch)}}}, {});
        },
        [](WorldNode*) {},
        [](LayerNode*) {}
      ));
    }
  }

  transaction.commit();
}

void AlignmentTool::distributeObjects(int axis, bool useSpacing, double spacing)
{
  auto& map = m_document.map();
  
  if (!map.selection().hasNodes())
  {
    return;
  }

  using namespace tb::mdl;

  auto nodes = map.selection().nodes;
  if (nodes.size() < 3)
  {
    return; // Need at least 3 objects to distribute
  }

  auto transaction = Transaction{map, "Distribute Objects"};

  // Sort objects by position along axis
  std::sort(nodes.begin(), nodes.end(), [axis](const Node* a, const Node* b) {
    return a->logicalBounds().center()[static_cast<size_t>(axis)] <
           b->logicalBounds().center()[static_cast<size_t>(axis)];
  });

  // Calculate distribution
  const auto firstBounds = nodes.front()->logicalBounds();
  const auto lastBounds = nodes.back()->logicalBounds();
  const double totalSpan = lastBounds.center()[static_cast<size_t>(axis)] -
                           firstBounds.center()[static_cast<size_t>(axis)];

  double step;
  if (useSpacing)
  {
    step = spacing;
  }
  else
  {
    step = totalSpan / static_cast<double>(nodes.size() - 1);
  }

  const double startPos = firstBounds.center()[static_cast<size_t>(axis)];

  // Move each object (skip first and last)
  for (size_t i = 1; i < nodes.size() - 1; ++i)
  {
    auto* node = nodes[i];
    const auto bounds = node->logicalBounds();
    const double targetPos = startPos + step * static_cast<double>(i);
    const double currentPos = bounds.center()[static_cast<size_t>(axis)];
    const double delta = targetPos - currentPos;

    if (std::abs(delta) > 0.001)
    {
      vm::vec3d translation{0, 0, 0};
      translation[static_cast<size_t>(axis)] = delta;
      
      const auto transform = vm::translation_matrix(translation);
      
      node->accept(kdl::overload(
        [&](BrushNode* brushNode) {
          auto brush = brushNode->brush();
          brush.transform(map.worldBounds(), transform, false);
          updateNodeContents(map, "Distribute", {{brushNode, NodeContents{std::move(brush)}}}, {});
        },
        [&](EntityNode* entityNode) {
          auto entity = entityNode->entity();
          entity.transform(transform, false);
          updateNodeContents(map, "Distribute", {{entityNode, NodeContents{std::move(entity)}}}, {});
        },
        [&](GroupNode* groupNode) {
          auto group = groupNode->group();
          group.transform(transform);
          updateNodeContents(map, "Distribute", {{groupNode, NodeContents{std::move(group)}}}, {});
        },
        [&](PatchNode* patchNode) {
          auto patch = patchNode->patch();
          patch.transform(transform);
          updateNodeContents(map, "Distribute", {{patchNode, NodeContents{std::move(patch)}}}, {});
        },
        [](WorldNode*) {},
        [](LayerNode*) {}
      ));
    }
  }

  transaction.commit();
}

void AlignmentTool::alignToGrid(int axis, AlignMode mode)
{
  auto& map = m_document.map();
  
  if (!map.selection().hasNodes())
  {
    return;
  }

  using namespace tb::mdl;

  auto transaction = Transaction{map, "Align to Grid"};

  const auto& gridRef = grid();
  
  for (auto* node : map.selection().nodes)
  {
    const auto bounds = node->logicalBounds();
    vm::vec3d translation{0, 0, 0};

    auto alignAxis = [&](int ax) {
      double currentPos = 0.0;
      switch (mode)
      {
      case AlignMode::Min:
        currentPos = bounds.min[static_cast<size_t>(ax)];
        break;
      case AlignMode::Center:
        currentPos = bounds.center()[static_cast<size_t>(ax)];
        break;
      case AlignMode::Max:
        currentPos = bounds.max[static_cast<size_t>(ax)];
        break;
      }

      const double snappedPos = gridRef.snap(currentPos);
      translation[static_cast<size_t>(ax)] = snappedPos - currentPos;
    };

    if (axis < 0)
    {
      // Align all axes
      alignAxis(0);
      alignAxis(1);
      alignAxis(2);
    }
    else
    {
      alignAxis(axis);
    }

    if (vm::length(translation) > 0.001)
    {
      const auto transform = vm::translation_matrix(translation);
      
      node->accept(kdl::overload(
        [&](BrushNode* brushNode) {
          auto brush = brushNode->brush();
          brush.transform(map.worldBounds(), transform, false);
          updateNodeContents(map, "Align to Grid", {{brushNode, NodeContents{std::move(brush)}}}, {});
        },
        [&](EntityNode* entityNode) {
          auto entity = entityNode->entity();
          entity.transform(transform, false);
          updateNodeContents(map, "Align to Grid", {{entityNode, NodeContents{std::move(entity)}}}, {});
        },
        [&](GroupNode* groupNode) {
          auto group = groupNode->group();
          group.transform(transform);
          updateNodeContents(map, "Align to Grid", {{groupNode, NodeContents{std::move(group)}}}, {});
        },
        [&](PatchNode* patchNode) {
          auto patch = patchNode->patch();
          patch.transform(transform);
          updateNodeContents(map, "Align to Grid", {{patchNode, NodeContents{std::move(patch)}}}, {});
        },
        [](WorldNode*) {},
        [](LayerNode*) {}
      ));
    }
  }

  transaction.commit();
}

void AlignmentTool::centerAround(const vm::vec3d& center)
{
  auto& map = m_document.map();
  
  if (!map.selection().hasNodes())
  {
    return;
  }

  using namespace tb::mdl;

  auto transaction = Transaction{map, "Center Around Point"};

  const auto selectionBounds = map.selection().selectionBounds();
  const auto selectionCenter = selectionBounds.center();
  const auto translation = center - selectionCenter;

  if (vm::length(translation) > 0.001)
  {
    const auto transform = vm::translation_matrix(translation);
    
    for (auto* node : map.selection().nodes)
    {
      node->accept(kdl::overload(
        [&](BrushNode* brushNode) {
          auto brush = brushNode->brush();
          brush.transform(map.worldBounds(), transform, false);
          updateNodeContents(map, "Center", {{brushNode, NodeContents{std::move(brush)}}}, {});
        },
        [&](EntityNode* entityNode) {
          auto entity = entityNode->entity();
          entity.transform(transform, false);
          updateNodeContents(map, "Center", {{entityNode, NodeContents{std::move(entity)}}}, {});
        },
        [&](GroupNode* groupNode) {
          auto group = groupNode->group();
          group.transform(transform);
          updateNodeContents(map, "Center", {{groupNode, NodeContents{std::move(group)}}}, {});
        },
        [&](PatchNode* patchNode) {
          auto patch = patchNode->patch();
          patch.transform(transform);
          updateNodeContents(map, "Center", {{patchNode, NodeContents{std::move(patch)}}}, {});
        },
        [](WorldNode*) {},
        [](LayerNode*) {}
      ));
    }
  }

  transaction.commit();
}

void AlignmentTool::stackObjects(int axis, double gap)
{
  auto& map = m_document.map();
  
  if (!map.selection().hasNodes())
  {
    return;
  }

  using namespace tb::mdl;

  auto nodes = map.selection().nodes;
  if (nodes.size() < 2)
  {
    return;
  }

  auto transaction = Transaction{map, "Stack Objects"};

  // Sort objects by position along axis
  std::sort(nodes.begin(), nodes.end(), [axis](const Node* a, const Node* b) {
    return a->logicalBounds().min[static_cast<size_t>(axis)] <
           b->logicalBounds().min[static_cast<size_t>(axis)];
  });

  // Stack each object on top of the previous
  double nextPos = nodes.front()->logicalBounds().max[static_cast<size_t>(axis)] + gap;

  for (size_t i = 1; i < nodes.size(); ++i)
  {
    auto* node = nodes[i];
    const auto bounds = node->logicalBounds();
    const double currentPos = bounds.min[static_cast<size_t>(axis)];
    const double delta = nextPos - currentPos;

    if (std::abs(delta) > 0.001)
    {
      vm::vec3d translation{0, 0, 0};
      translation[static_cast<size_t>(axis)] = delta;
      
      const auto transform = vm::translation_matrix(translation);
      
      node->accept(kdl::overload(
        [&](BrushNode* brushNode) {
          auto brush = brushNode->brush();
          brush.transform(map.worldBounds(), transform, false);
          updateNodeContents(map, "Stack", {{brushNode, NodeContents{std::move(brush)}}}, {});
        },
        [&](EntityNode* entityNode) {
          auto entity = entityNode->entity();
          entity.transform(transform, false);
          updateNodeContents(map, "Stack", {{entityNode, NodeContents{std::move(entity)}}}, {});
        },
        [&](GroupNode* groupNode) {
          auto group = groupNode->group();
          group.transform(transform);
          updateNodeContents(map, "Stack", {{groupNode, NodeContents{std::move(group)}}}, {});
        },
        [&](PatchNode* patchNode) {
          auto patch = patchNode->patch();
          patch.transform(transform);
          updateNodeContents(map, "Stack", {{patchNode, NodeContents{std::move(patch)}}}, {});
        },
        [](WorldNode*) {},
        [](LayerNode*) {}
      ));
    }

    // Update nextPos for the moved object
    nextPos = bounds.max[static_cast<size_t>(axis)] + delta + gap;
  }

  transaction.commit();
}

QWidget* AlignmentTool::doCreatePage(QWidget* parent)
{
  return new AlignmentToolPage{m_document, *this, parent};
}

} // namespace tb::ui
