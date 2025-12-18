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

#include "ArrayTool.h"
#include "ArrayToolPage.h"

#include "mdl/Brush.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_CopyPaste.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
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
#include "kd/ranges/to.h"

#include <map>
#include <vector>
#include <ranges>
#include <cmath>

namespace tb::ui
{

ArrayTool::ArrayTool(MapDocument& document)
  : Tool{false}
  , m_document{document}
{
}

bool ArrayTool::doActivate()
{
  return true;
}

const mdl::Grid& ArrayTool::grid() const
{
  return m_document.map().grid();
}

void ArrayTool::applyLinearArray(
  int count,
  const vm::vec3d& offset,
  bool groupResult)
{
  auto& map = m_document.map();
  
  if (!map.selection().hasNodes() || count <= 0)
  {
    return;
  }

  using namespace tb::mdl;

  auto transaction = Transaction{map, "Linear Array"};

  // Get the selected nodes to duplicate
  auto nodesToDuplicate = map.selection().nodes;
  std::vector<Node*> allNewNodes;

  for (int i = 1; i <= count; ++i)
  {
    // Calculate transform for this copy
    const vm::vec3d translation = offset * static_cast<double>(i);
    const auto transform = vm::translation_matrix(translation);

    // Duplicate and transform each selected node
    for (auto* node : nodesToDuplicate)
    {
      node->accept(kdl::overload(
        [&](GroupNode* groupNode) {
          auto group = groupNode->group();
          group.transform(transform);
          
          // Clone the group node
          auto* newGroupNode = new GroupNode{std::move(group)};
          
          // Clone children recursively
          groupNode->visitChildren([&](Node* child) {
            child->accept(kdl::overload(
              [&](BrushNode* brushNode) {
                auto brush = brushNode->brush();
                (void)brush.transform(map.worldBounds(), transform, false);
                auto* newBrushNode = new BrushNode{std::move(brush)};
                newGroupNode->addChild(newBrushNode);
              },
              [&](EntityNode* entityNode) {
                auto entity = entityNode->entity();
                entity.transform(transform, false);
                auto* newEntityNode = new EntityNode{std::move(entity)};
                newGroupNode->addChild(newEntityNode);
              },
              [&](PatchNode* patchNode) {
                auto patch = patchNode->patch();
                patch.transform(transform);
                auto* newPatchNode = new PatchNode{std::move(patch)};
                newGroupNode->addChild(newPatchNode);
              },
              [](WorldNode*) {},
              [](LayerNode*) {},
              [](GroupNode*) {} // Nested groups handled separately
            ));
          });
          
          addNodes(map, {{node->parent(), {newGroupNode}}});
          allNewNodes.push_back(newGroupNode);
        },
        [&](BrushNode* brushNode) {
          auto brush = brushNode->brush();
          auto result = brush.transform(map.worldBounds(), transform, false);
          if (result.is_success())
          {
            auto* newBrushNode = new BrushNode{std::move(brush)};
            addNodes(map, {{node->parent(), {newBrushNode}}});
            allNewNodes.push_back(newBrushNode);
          }
        },
        [&](EntityNode* entityNode) {
          auto entity = entityNode->entity();
          entity.transform(transform, false);
          auto* newEntityNode = new EntityNode{std::move(entity)};
          addNodes(map, {{node->parent(), {newEntityNode}}});
          allNewNodes.push_back(newEntityNode);
          
          // Also duplicate any child brushes
          entityNode->visitChildren([&](Node* child) {
            child->accept(kdl::overload(
              [&](BrushNode* childBrush) {
                auto brush = childBrush->brush();
                (void)brush.transform(map.worldBounds(), transform, false);
                auto* newBrushNode = new BrushNode{std::move(brush)};
                newEntityNode->addChild(newBrushNode);
              },
              [](BrushNode*) {},
              [](EntityNode*) {},
              [](PatchNode*) {},
              [](GroupNode*) {},
              [](WorldNode*) {},
              [](LayerNode*) {}
            ));
          });
        },
        [&](PatchNode* patchNode) {
          auto patch = patchNode->patch();
          patch.transform(transform);
          auto* newPatchNode = new PatchNode{std::move(patch)};
          addNodes(map, {{node->parent(), {newPatchNode}}});
          allNewNodes.push_back(newPatchNode);
        },
        [](WorldNode*) {},
        [](LayerNode*) {}
      ));
    }
  }

  if (groupResult && !allNewNodes.empty())
  {
    // Group the new nodes together with originals
    std::vector<Node*> toGroup = nodesToDuplicate;
    toGroup.insert(toGroup.end(), allNewNodes.begin(), allNewNodes.end());
    selectNodes(map, toGroup);
    groupSelectedNodes(map, "Array");
  }
  else if (!allNewNodes.empty())
  {
    // Select the new nodes
    deselectAll(map);
    selectNodes(map, allNewNodes);
  }

  transaction.commit();
}

void ArrayTool::applyGridArray(
  int countX, int countY, int countZ,
  double spacingX, double spacingY, double spacingZ,
  bool groupResult)
{
  auto& map = m_document.map();
  
  if (!map.selection().hasNodes())
  {
    return;
  }

  // Calculate the bounding box of selected objects for spacing
  const auto bounds = *map.selectionBounds();
  const auto size = bounds.size();

  // Add object size to spacing for proper separation
  const double totalSpacingX = size.x() + spacingX;
  const double totalSpacingY = size.y() + spacingY;
  const double totalSpacingZ = size.z() + spacingZ;

  using namespace tb::mdl;

  auto transaction = Transaction{map, "Grid Array"};

  auto nodesToDuplicate = map.selection().nodes;
  std::vector<Node*> allNewNodes;

  for (int x = 0; x < countX; ++x)
  {
    for (int y = 0; y < countY; ++y)
    {
      for (int z = 0; z < countZ; ++z)
      {
        // Skip the original position (0,0,0)
        if (x == 0 && y == 0 && z == 0)
        {
          continue;
        }

        const vm::vec3d translation{
          static_cast<double>(x) * totalSpacingX,
          static_cast<double>(y) * totalSpacingY,
          static_cast<double>(z) * totalSpacingZ
        };
        const auto transform = vm::translation_matrix(translation);

        for (auto* node : nodesToDuplicate)
        {
          node->accept(kdl::overload(
            [&](BrushNode* brushNode) {
              auto brush = brushNode->brush();
              auto result = brush.transform(map.worldBounds(), transform, false);
              if (result.is_success())
              {
                auto* newBrushNode = new BrushNode{std::move(brush)};
                addNodes(map, {{node->parent(), {newBrushNode}}});
                allNewNodes.push_back(newBrushNode);
              }
            },
            [&](EntityNode* entityNode) {
              auto entity = entityNode->entity();
              entity.transform(transform, false);
              auto* newEntityNode = new EntityNode{std::move(entity)};
              addNodes(map, {{node->parent(), {newEntityNode}}});
              allNewNodes.push_back(newEntityNode);
            },
            [&](PatchNode* patchNode) {
              auto patch = patchNode->patch();
              patch.transform(transform);
              auto* newPatchNode = new PatchNode{std::move(patch)};
              addNodes(map, {{node->parent(), {newPatchNode}}});
              allNewNodes.push_back(newPatchNode);
            },
            [&](GroupNode* groupNode) {
              auto group = groupNode->group();
              group.transform(transform);
              auto* newGroupNode = new GroupNode{std::move(group)};
              addNodes(map, {{node->parent(), {newGroupNode}}});
              allNewNodes.push_back(newGroupNode);
            },
            [](WorldNode*) {},
            [](LayerNode*) {}
          ));
        }
      }
    }
  }

  if (groupResult && !allNewNodes.empty())
  {
    std::vector<Node*> toGroup = nodesToDuplicate;
    toGroup.insert(toGroup.end(), allNewNodes.begin(), allNewNodes.end());
    selectNodes(map, toGroup);
    groupSelectedNodes(map, "Grid Array");
  }
  else if (!allNewNodes.empty())
  {
    deselectAll(map);
    selectNodes(map, allNewNodes);
  }

  transaction.commit();
}

void ArrayTool::applyRadialArray(
  int count,
  const vm::vec3d& center,
  int axis,
  double totalAngle,
  bool groupResult)
{
  auto& map = m_document.map();
  
  if (!map.selection().hasNodes() || count <= 0)
  {
    return;
  }

  using namespace tb::mdl;

  auto transaction = Transaction{map, "Radial Array"};

  auto nodesToDuplicate = map.selection().nodes;
  std::vector<Node*> allNewNodes;

  const double angleIncrement = vm::to_radians(totalAngle) / static_cast<double>(count);
  const auto axisVec = vm::vec3d::axis(static_cast<size_t>(axis));

  for (int i = 1; i <= count; ++i)
  {
    const double angle = angleIncrement * static_cast<double>(i);
    
    // Create rotation transform around center point
    const auto transform = 
      vm::translation_matrix(center) *
      vm::rotation_matrix(axisVec, angle) *
      vm::translation_matrix(-center);

    for (auto* node : nodesToDuplicate)
    {
      node->accept(kdl::overload(
        [&](BrushNode* brushNode) {
          auto brush = brushNode->brush();
          auto result = brush.transform(map.worldBounds(), transform, false);
          if (result.is_success())
          {
            auto* newBrushNode = new BrushNode{std::move(brush)};
            addNodes(map, {{node->parent(), {newBrushNode}}});
            allNewNodes.push_back(newBrushNode);
          }
        },
        [&](EntityNode* entityNode) {
          auto entity = entityNode->entity();
          entity.transform(transform, false);
          auto* newEntityNode = new EntityNode{std::move(entity)};
          addNodes(map, {{node->parent(), {newEntityNode}}});
          allNewNodes.push_back(newEntityNode);
        },
        [&](PatchNode* patchNode) {
          auto patch = patchNode->patch();
          patch.transform(transform);
          auto* newPatchNode = new PatchNode{std::move(patch)};
          addNodes(map, {{node->parent(), {newPatchNode}}});
          allNewNodes.push_back(newPatchNode);
        },
        [&](GroupNode* groupNode) {
          auto group = groupNode->group();
          group.transform(transform);
          auto* newGroupNode = new GroupNode{std::move(group)};
          addNodes(map, {{node->parent(), {newGroupNode}}});
          allNewNodes.push_back(newGroupNode);
        },
        [](WorldNode*) {},
        [](LayerNode*) {}
      ));
    }
  }

  if (groupResult && !allNewNodes.empty())
  {
    std::vector<Node*> toGroup = nodesToDuplicate;
    toGroup.insert(toGroup.end(), allNewNodes.begin(), allNewNodes.end());
    selectNodes(map, toGroup);
    groupSelectedNodes(map, "Radial Array");
  }
  else if (!allNewNodes.empty())
  {
    deselectAll(map);
    selectNodes(map, allNewNodes);
  }

  transaction.commit();
}

QWidget* ArrayTool::doCreatePage(QWidget* parent)
{
  return new ArrayToolPage{m_document, *this, parent};
}

} // namespace tb::ui
