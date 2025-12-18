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

#pragma once

#include "vm/vec.h"
#include "vm/bbox.h"

#include <optional>
#include <vector>

namespace tb::mdl
{
class Node;
}

namespace tb::ui
{

/**
 * Smart snapping modes for enhanced positioning
 */
enum class SmartSnapMode
{
  /** Snap to grid only */
  GridOnly,
  
  /** Snap to other brush vertices */
  Vertices,
  
  /** Snap to other brush edges */
  Edges,
  
  /** Snap to other brush face centers */
  FaceCenters,
  
  /** Snap to other object centers */
  ObjectCenters,
  
  /** Snap to any nearby geometry */
  All
};

/**
 * Smart snapping utility that provides intelligent snapping to nearby geometry
 */
class SmartSnap
{
public:
  struct SnapResult
  {
    vm::vec3d position;
    SmartSnapMode mode;
    std::optional<const mdl::Node*> snappedTo;
    double distance;
  };

  /**
   * Find the best snap target for the given position
   * @param position Current position to snap
   * @param gridSize Current grid size
   * @param mode Snapping mode to use
   * @param candidates Nearby nodes to consider for snapping
   * @param snapThreshold Maximum distance to snap
   */
  static std::optional<SnapResult> findSnapTarget(
    const vm::vec3d& position,
    double gridSize,
    SmartSnapMode mode,
    const std::vector<const mdl::Node*>& candidates,
    double snapThreshold = 8.0);

  /**
   * Snap position to grid
   */
  static vm::vec3d snapToGrid(const vm::vec3d& position, double gridSize);

  /**
   * Find nearest vertex in candidates
   */
  static std::optional<SnapResult> snapToVertex(
    const vm::vec3d& position,
    const std::vector<const mdl::Node*>& candidates,
    double threshold);

  /**
   * Find nearest edge in candidates
   */
  static std::optional<SnapResult> snapToEdge(
    const vm::vec3d& position,
    const std::vector<const mdl::Node*>& candidates,
    double threshold);

  /**
   * Find nearest object center
   */
  static std::optional<SnapResult> snapToCenter(
    const vm::vec3d& position,
    const std::vector<const mdl::Node*>& candidates,
    double threshold);
};

} // namespace tb::ui
