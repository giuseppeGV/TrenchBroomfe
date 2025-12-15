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

#include "EdgeTool.h"

#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"
#include "kd/string_format.h"

namespace tb::ui
{

EdgeTool::EdgeTool(MapDocument& document)
  : VertexToolBase{document}
{
}

void EdgeTool::setBevelMode(bool enabled)
{
  m_bevelMode = enabled;
}

bool EdgeTool::bevelMode() const
{
  return m_bevelMode;
}

std::vector<mdl::BrushNode*> EdgeTool::findIncidentBrushes(
  const vm::segment3d& handle) const
{
  return findIncidentBrushes(handleManager(), handle);
}

void EdgeTool::pick(
  const vm::ray3d& pickRay,
  const render::Camera& camera,
  mdl::PickResult& pickResult) const
{
  handleManager().pickCenterHandle(pickRay, camera, pickResult);
}

mdl::EdgeHandleManager& EdgeTool::handleManager()
{
  return m_document.map().edgeHandles();
}

const mdl::EdgeHandleManager& EdgeTool::handleManager() const
{
  return m_document.map().edgeHandles();
}

std::tuple<vm::vec3d, vm::vec3d> EdgeTool::handlePositionAndHitPoint(
  const std::vector<mdl::Hit>& hits) const
{
  contract_pre(!hits.empty());

  const auto& hit = hits.front();
  contract_assert(hit.hasType(mdl::EdgeHandleManager::HandleHitType));

  return {hit.target<vm::segment3d>().center(), hit.hitPoint()};
}

bool EdgeTool::startMove(const std::vector<mdl::Hit>& hits)
{
  if (!VertexToolBase::startMove(hits))
  {
    return false;
  }

  if (m_bevelMode)
  {
    m_initialBrushes.clear();
    m_totalDelta = vm::vec3d::zero();

    const auto handles = handleManager().selectedHandles();
    for (auto* brushNode : findIncidentBrushes(handleManager(), handles))
    {
      m_initialBrushes.emplace(brushNode, brushNode->brush());
    }
  }

  return true;
}

EdgeTool::MoveResult EdgeTool::move(const vm::vec3d& delta)
{
  auto& map = m_document.map();

  if (m_bevelMode)
  {
    m_totalDelta += delta;
    
    // Determine bevel amount (distance).
    // Simple heuristic: length of delta, or project onto some direction.
    // For now, let's use the magnitude, but check direction relative to something?
    // Actually, simple dragging distance is fine for a first pass.
    // We can refine 'distance' calculation later (e.g. project on view plane normal etc).
    double distance = vm::length(m_totalDelta);
    
    if (distance < vm::Cd::almost_zero())
    {
        return MoveResult::Continue;
    }

    // Apply bevel to all affected brushes
    for (auto& [node, initialBrush] : m_initialBrushes)
    {
       // Start with fresh copy
       auto brush = initialBrush; // Copy
       
       // Apply bevel to ALL selected edges incident to this brush
       // We need to map the selected edges (which are segments) to the brush edges.
       // The handle IS the edge segment.
       // Brush::bevelEdge takes a segment.
       // We should try to apply it.
       // Note: applying multiple bevels to one brush:
       // If we bevel edge A, topology changes. Edge B might still exist or be modified.
       // Using `segment3d` to identify edge relies on geometry matching.
       // If we process multiple edges, we must be careful.
       
       // For this iteration, let's just attempt to bevel all selected edges provided they exist in the brush.
       // We iterate selected handles.
       
       auto handles = map.edgeHandles().selectedHandles();
       for (const auto& edgeHandle : handles)
       {
           // Check if this edge belongs to the INITIAL brush (geometry match)
           // We use the initial brush geometry to check 'validity' but we apply to the 'current' brush (accumulating cuts).
           // Actually, we should apply strictly to the `brush` variable we just copied.
           
           // Issue: `bevelEdge` modifies `brush`. The next `bevelEdge` call on same brush needs to find the other edge.
           // Since `bevelEdge` only cuts a corner, other edges likely remain intact in terms of position (if they are far enough).
           // `Brush::bevelEdge` uses `findEdgeByPositions` with epsilon.
           
           if (initialBrush.hasEdge(edgeHandle, 0.1)) // Loose check on initial to see if relevant
           {
               // Try to bevel
               // We ignore error if edge not found (might have been consumed by previous bevel?)
               brush.bevelEdge(map.worldNode().mapFormat(), map.worldBounds(), edgeHandle, distance, false);
           }
       }
       
       node->setBrush(std::move(brush));
    }
    
    return MoveResult::Continue;
  }
  else
  {
      auto handles = map.edgeHandles().selectedHandles();
      const auto transform = vm::translation_matrix(delta);
      if (transformEdges(map, std::move(handles), transform))
      {
        m_dragHandlePosition = m_dragHandlePosition.transform(transform);
        return MoveResult::Continue;
      }
      return MoveResult::Deny;
  }
}

void EdgeTool::endMove()
{
  VertexToolBase::endMove();
  m_initialBrushes.clear();
}

void EdgeTool::cancelMove()
{
  VertexToolBase::cancelMove();
  m_initialBrushes.clear();
}

std::string EdgeTool::actionName() const
{
  if (m_bevelMode)
  {
      return "Bevel Edge";
  }
  return kdl::str_plural(
    handleManager().selectedHandleCount(), "Move Edge", "Move Edges");
}

void EdgeTool::removeSelection()
{
  auto& map = m_document.map();

  const auto handles = map.edgeHandles().selectedHandles();
  auto vertexPositions = std::vector<vm::vec3d>{};
  vertexPositions.reserve(2 * vertexPositions.size());
  vm::segment3d::get_vertices(
    std::begin(handles), std::end(handles), std::back_inserter(vertexPositions));

  const auto commandName =
    kdl::str_plural(handles.size(), "Remove Brush Edge", "Remove Brush Edges");
  removeVertices(map, commandName, std::move(vertexPositions));
}

namespace
{
bool areSegmentsEqual(const vm::segment3d& a, const vm::segment3d& b, double epsilon)
{
  return (vm::distance_sq(a.start, b.start) < epsilon * epsilon && vm::distance_sq(a.end, b.end) < epsilon * epsilon)
         || (vm::distance_sq(a.start, b.end) < epsilon * epsilon && vm::distance_sq(a.end, b.start) < epsilon * epsilon);
}
}

void EdgeTool::selectFaceLoop(const vm::segment3d& edge)
{
  auto brushes = findIncidentBrushes(edge);
  auto selection = handleManager().selection();

  for (auto* brushNode : brushes)
  {
    if (!brushNode) continue;
    const auto& brush = brushNode->brush();
    for (const auto& face : brush.faces())
    {
      bool hasEdge = false;
      // Check if face has this edge.
      for (const auto* faceEdge : face.edges())
      {
        if (areSegmentsEqual(faceEdge->segment(), edge, 0.1))
        {
          hasEdge = true;
          break;
        }
      }

      if (hasEdge)
      {
        // Select all edges of this face
        for (const auto* faceEdge : face.edges())
        {
          selection.insert(faceEdge->segment());
        }
      }
    }
  }
  handleManager().setSelection(selection);
}

} // namespace tb::ui

