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

#include "ui/Tool.h"

#include "vm/vec.h"

namespace tb
{
namespace mdl
{
class Grid;
}

namespace ui
{
class MapDocument;

/**
 * Array mode determines how objects are duplicated
 */
enum class ArrayMode
{
  Linear,  // Single axis duplication
  Grid,    // 2D/3D grid duplication
  Radial   // Circular pattern around an axis
};

/**
 * Tool for creating arrays/clones of selected objects in various patterns.
 * Supports linear, grid, and radial array modes.
 */
class ArrayTool : public Tool
{
private:
  MapDocument& m_document;

public:
  explicit ArrayTool(MapDocument& document);

  bool doActivate() override;

  const mdl::Grid& grid() const;

  /**
   * Creates a linear array of the selected objects.
   * 
   * @param count Number of copies to create
   * @param offset Offset between each copy
   * @param groupResult If true, groups all resulting objects
   */
  void applyLinearArray(
    int count,
    const vm::vec3d& offset,
    bool groupResult);

  /**
   * Creates a grid array of the selected objects.
   * 
   * @param countX Number of copies in X direction
   * @param countY Number of copies in Y direction
   * @param countZ Number of copies in Z direction
   * @param spacingX Spacing between copies in X direction
   * @param spacingY Spacing between copies in Y direction
   * @param spacingZ Spacing between copies in Z direction
   * @param groupResult If true, groups all resulting objects
   */
  void applyGridArray(
    int countX, int countY, int countZ,
    double spacingX, double spacingY, double spacingZ,
    bool groupResult);

  /**
   * Creates a radial array of the selected objects.
   * 
   * @param count Number of copies to create around the circle
   * @param center Center point for the radial pattern
   * @param axis Axis to rotate around (0=X, 1=Y, 2=Z)
   * @param totalAngle Total angle to spread copies over (360 for full circle)
   * @param groupResult If true, groups all resulting objects
   */
  void applyRadialArray(
    int count,
    const vm::vec3d& center,
    int axis,
    double totalAngle,
    bool groupResult);

private:
  QWidget* doCreatePage(QWidget* parent) override;
};

} // namespace ui
} // namespace tb
