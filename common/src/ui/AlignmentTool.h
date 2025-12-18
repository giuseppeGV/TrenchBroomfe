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
#include "vm/bbox.h"

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
 * Alignment modes for aligning objects
 */
enum class AlignMode
{
  Min,     // Align to minimum (left/bottom/back)
  Center,  // Align to center
  Max      // Align to maximum (right/top/front)
};

/**
 * Tool for aligning and distributing selected objects.
 */
class AlignmentTool : public Tool
{
private:
  MapDocument& m_document;

public:
  explicit AlignmentTool(MapDocument& document);

  bool doActivate() override;

  const mdl::Grid& grid() const;

  /**
   * Aligns selected objects along the specified axis.
   * 
   * @param axis The axis to align along (0=X, 1=Y, 2=Z)
   * @param mode The alignment mode (Min, Center, Max)
   * @param alignToFirst If true, aligns all objects to the first selected object
   */
  void alignObjects(int axis, AlignMode mode, bool alignToFirst);

  /**
   * Distributes selected objects evenly along the specified axis.
   * 
   * @param axis The axis to distribute along (0=X, 1=Y, 2=Z)
   * @param useSpacing If true, uses the spacing value; otherwise distributes evenly
   * @param spacing The spacing between objects (if useSpacing is true)
   */
  void distributeObjects(int axis, bool useSpacing, double spacing);

  /**
   * Aligns selected objects to the grid.
   * 
   * @param axis The axis to align to grid (0=X, 1=Y, 2=Z, or -1 for all)
   * @param mode The alignment mode (Min, Center, Max)
   */
  void alignToGrid(int axis, AlignMode mode);

  /**
   * Centers selected objects around a specific point.
   * 
   * @param center The center point
   */
  void centerAround(const vm::vec3d& center);

  /**
   * Stacks objects on top of each other (useful for stairs, etc.)
   * 
   * @param axis The axis to stack along (0=X, 1=Y, 2=Z)
   * @param gap The gap between stacked objects
   */
  void stackObjects(int axis, double gap);

private:
  QWidget* doCreatePage(QWidget* parent) override;
};

} // namespace ui
} // namespace tb
