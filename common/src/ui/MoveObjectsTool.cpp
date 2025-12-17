/*
 Copyright (C) 2010 Kristian Duske

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

#include "MoveObjectsTool.h"

#include "mdl/BrushNode.h"
#include "mdl/SymmetryManager.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include <algorithm>
#include <vector>

#include "mdl/Grid.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Nodes.h"
#include "mdl/TransactionScope.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "vm/bbox.h"

namespace
{

void translateNodes(const std::vector<tb::mdl::Node*>& nodes, const vm::vec3d& delta, const vm::bbox3d& worldBounds)
{
    using namespace tb::mdl;
    for (auto* node : nodes)
    {
        if (auto* brushNode = dynamic_cast<BrushNode*>(node))
        {
             auto brush = brushNode->brush();
             [[maybe_unused]] auto result = brush.transform(worldBounds, vm::translation_matrix(delta), true);
             brushNode->setBrush(std::move(brush));
        }
    }
}

std::vector<tb::mdl::Node*> findSymmetricNodes(tb::mdl::Map& map, const tb::mdl::SymmetryManager& sm)
{
    using namespace tb::mdl;
    std::vector<Node*> result;
    const auto& selection = map.selection();
    
    // Naive search: iterate all nodes in map.
    auto allNodes = map.findNodes<Node>("*");
    
    for (auto* selectedNode : selection.nodes)
    {
        vm::vec3d center = selectedNode->logicalBounds().center();
        vm::vec3d target = sm.reflect(center);
        
        for (auto* candidate : allNodes)
        {
            if (candidate == selectedNode) continue;
            if (std::find(selection.nodes.begin(), selection.nodes.end(), candidate) != selection.nodes.end()) continue;
            
            if (vm::length(candidate->logicalBounds().center() - target) < 2.0)
            {
                result.push_back(candidate);
                break; 
            }
        }
    }
    return result;
}

}




#include <cassert>

namespace tb::ui
{

MoveObjectsTool::MoveObjectsTool(MapDocument& document)
  : Tool{true}
  , m_document{document}
{
}

const mdl::Grid& MoveObjectsTool::grid() const
{
  return m_document.map().grid();
}

bool MoveObjectsTool::startMove(const InputState& inputState)
{
  auto& map = m_document.map();

  m_symmetricNodes.clear(); 

  if (!map.selection().brushFaces.empty())
  {
    return false;
  }

  if (m_document.symmetryManager().isEnabled())
  {
      m_symmetricNodes = findSymmetricNodes(map, m_document.symmetryManager());
  }

  map.startTransaction(
    duplicateObjects(inputState) ? "Duplicate Objects" : "Move Objects",
    mdl::TransactionScope::LongRunning);
  m_duplicateObjects = duplicateObjects(inputState);
  return true;
}

MoveObjectsTool::MoveResult MoveObjectsTool::move(
  const InputState&, const vm::vec3d& delta)
{
  auto& map = m_document.map();

  const auto& worldBounds = map.worldBounds();
  const auto bounds = map.selectionBounds();
  if (!bounds)
  {
    return MoveResult::Cancel;
  }

  if (!worldBounds.contains(bounds->translate(delta)))
  {
    return MoveResult::Deny;
  }

  if (m_duplicateObjects)
  {
    m_duplicateObjects = false;
    duplicateSelectedNodes(map);
  }

  if (!m_symmetricNodes.empty())
  {
      vm::vec3d reflectedDelta = m_document.symmetryManager().reflectVector(delta);
      translateNodes(m_symmetricNodes, reflectedDelta, worldBounds);
  }

  return translateSelection(map, delta) ? MoveResult::Continue : MoveResult::Deny;
}

void MoveObjectsTool::endMove(const InputState&)
{
  m_document.map().commitTransaction();
}

void MoveObjectsTool::cancelMove()
{
  m_document.map().cancelTransaction();
}

bool MoveObjectsTool::duplicateObjects(const InputState& inputState) const
{
  return inputState.modifierKeysDown(ModifierKeys::CtrlCmd);
}

} // namespace tb::ui
