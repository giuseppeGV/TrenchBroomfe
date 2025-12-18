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

#include <optional>
#include <vector>

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
 * Measurement result containing dimension info
 */
struct MeasurementResult
{
  double distanceX = 0.0;
  double distanceY = 0.0;
  double distanceZ = 0.0;
  double totalDistance = 0.0;
  double area = 0.0;
  double volume = 0.0;
  int brushCount = 0;
  int entityCount = 0;
  int faceCount = 0;
  int vertexCount = 0;
};

/**
 * Tool for measuring distances, areas, and volumes in the map.
 */
class MeasureTool : public Tool
{
private:
  MapDocument& m_document;
  std::optional<vm::vec3d> m_measureStartPoint;
  std::optional<vm::vec3d> m_measureEndPoint;

public:
  explicit MeasureTool(MapDocument& document);

  bool doActivate() override;

  const mdl::Grid& grid() const;

  /**
   * Sets the start point for distance measurement.
   */
  void setStartPoint(const vm::vec3d& point);

  /**
   * Sets the end point for distance measurement.
   */
  void setEndPoint(const vm::vec3d& point);

  /**
   * Clears the measurement points.
   */
  void clearMeasurement();

  /**
   * Gets the current start point.
   */
  std::optional<vm::vec3d> startPoint() const;

  /**
   * Gets the current end point.
   */
  std::optional<vm::vec3d> endPoint() const;

  /**
   * Calculates the distance between start and end points.
   */
  double calculateDistance() const;

  /**
   * Gets component distances (X, Y, Z).
   */
  vm::vec3d calculateComponentDistances() const;

  /**
   * Measures the selection and returns detailed results.
   */
  MeasurementResult measureSelection() const;

  /**
   * Gets the bounding box dimensions of the selection.
   */
  vm::vec3d selectionDimensions() const;

  /**
   * Calculates the total face area of selected brushes.
   */
  double calculateTotalFaceArea() const;

  /**
   * Calculates the approximate volume of selected brushes.
   */
  double calculateTotalVolume() const;

private:
  QWidget* doCreatePage(QWidget* parent) override;
};

} // namespace ui
} // namespace tb
