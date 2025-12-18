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

#pragma once

#include "BrushFaceAttributes.h"
#include "Result.h"
#include "mdl/CircleShape.h"
#include "mdl/Polyhedron3.h"

#include "vm/bbox.h"
#include "vm/util.h"

#include <string>
#include <vector>

namespace tb::mdl
{
class Brush;
class ModelFactory;
enum class MapFormat;

class BrushBuilder
{
private:
  MapFormat m_mapFormat;
  const vm::bbox3d m_worldBounds;
  const BrushFaceAttributes m_defaultAttribs;

public:
  BrushBuilder(MapFormat mapFormat, const vm::bbox3d& worldBounds);
  BrushBuilder(
    MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    BrushFaceAttributes defaultAttribs);

  Result<Brush> createCube(double size, const std::string& materialName) const;
  Result<Brush> createCube(
    double size,
    const std::string& leftMaterial,
    const std::string& rightMaterial,
    const std::string& frontMaterial,
    const std::string& backMaterial,
    const std::string& topMaterial,
    const std::string& bottomMaterial) const;

  Result<Brush> createCuboid(
    const vm::vec3d& size, const std::string& materialName) const;
  Result<Brush> createCuboid(
    const vm::vec3d& size,
    const std::string& leftMaterial,
    const std::string& rightMaterial,
    const std::string& frontMaterial,
    const std::string& backMaterial,
    const std::string& topMaterial,
    const std::string& bottomMaterial) const;

  Result<Brush> createCuboid(
    const vm::bbox3d& bounds, const std::string& materialName) const;
  Result<Brush> createCuboid(
    const vm::bbox3d& bounds,
    const std::string& leftMaterial,
    const std::string& rightMaterial,
    const std::string& frontMaterial,
    const std::string& backMaterial,
    const std::string& topMaterial,
    const std::string& bottomMaterial) const;

  Result<Brush> createCylinder(
    const vm::bbox3d& bounds,
    const CircleShape& circleShape,
    vm::axis::type axis,
    const std::string& textureName) const;

  Result<std::vector<Brush>> createHollowCylinder(
    const vm::bbox3d& bounds,
    double thickness,
    const CircleShape& circleShape,
    vm::axis::type axis,
    const std::string& textureName) const;

  Result<Brush> createScalableCylinder(
    const vm::bbox3d& bounds,
    size_t precision,
    vm::axis::type axis,
    const std::string& textureName) const;

  Result<Brush> createCone(
    const vm::bbox3d& bounds,
    const CircleShape& circleShape,
    vm::axis::type axis,
    const std::string& textureName) const;

  Result<Brush> createUVSphere(
    const vm::bbox3d& bounds,
    const CircleShape& circleShape,
    size_t numRings,
    vm::axis::type axis,
    const std::string& textureName) const;

  Result<Brush> createIcoSphere(
    const vm::bbox3d& bounds, size_t iterations, const std::string& textureName) const;

  Result<Brush> createBrush(
    const std::vector<vm::vec3d>& points, const std::string& materialName) const;
  Result<Brush> createBrush(
    const Polyhedron3& polyhedron, const std::string& materialName) const;

  /**
   * Creates a single stair step brush
   */
  Result<Brush> createStairStep(
    const vm::vec3d& position,
    const vm::vec3d& stepSize,
    const std::string& materialName) const;

  /**
   * Creates multiple stair brushes
   * @param bounds The bounding box for the entire staircase
   * @param stepCount Number of steps
   * @param axis Direction of stair ascent (0=X, 1=Y, 2=Z for up direction)
   * @param direction 1 for ascending in positive axis, -1 for negative
   */
  Result<std::vector<Brush>> createStairs(
    const vm::bbox3d& bounds,
    size_t stepCount,
    vm::axis::type axis,
    int direction,
    const std::string& materialName) const;

  /**
   * Creates an arch shape from multiple brushes
   * @param bounds The bounding box for the arch
   * @param thickness Wall thickness
   * @param circleShape Shape parameters for the arch curve
   * @param axis Axis along which the arch opens
   */
  Result<std::vector<Brush>> createArch(
    const vm::bbox3d& bounds,
    double thickness,
    const CircleShape& circleShape,
    vm::axis::type axis,
    const std::string& materialName) const;

  /**
   * Creates a spiral staircase
   * @param center Center point of the spiral
   * @param innerRadius Inner radius of the spiral
   * @param outerRadius Outer radius of the spiral
   * @param height Total height of the spiral
   * @param stepCount Number of steps
   * @param rotations Total rotations (1.0 = 360 degrees)
   */
  Result<std::vector<Brush>> createSpiralStairs(
    const vm::vec3d& center,
    double innerRadius,
    double outerRadius,
    double height,
    size_t stepCount,
    double rotations,
    const std::string& materialName) const;
};

} // namespace tb::mdl
