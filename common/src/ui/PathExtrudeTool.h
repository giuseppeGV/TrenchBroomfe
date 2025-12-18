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

#include <vector>

namespace tb
{
namespace mdl
{
class BrushNode;
}

namespace ui
{
class MapDocument;

/**
 * Tool for extruding a brush along a path defined by waypoints.
 */
class PathExtrudeTool : public Tool
{
private:
  MapDocument& m_document;
  
  // Path waypoints
  std::vector<vm::vec3d> m_waypoints;
  
  // Settings
  int m_segments = 4;
  bool m_alignToPath = true;
  bool m_scaleAlongPath = false;
  double m_startScale = 1.0;
  double m_endScale = 1.0;
  bool m_twist = false;
  double m_twistAngle = 0.0; // Degrees per segment

public:
  explicit PathExtrudeTool(MapDocument& document);

  bool doActivate() override;
  bool doDeactivate() override;

  // Path manipulation
  void addWaypoint(const vm::vec3d& point);
  void removeLastWaypoint();
  void clearWaypoints();
  size_t waypointCount() const;
  const std::vector<vm::vec3d>& waypoints() const;

  // Settings
  void setSegments(int segments);
  int segments() const;

  void setAlignToPath(bool align);
  bool alignToPath() const;

  void setScaleAlongPath(bool scale);
  bool scaleAlongPath() const;

  void setStartScale(double scale);
  double startScale() const;

  void setEndScale(double scale);
  double endScale() const;

  void setTwist(bool twist);
  bool twist() const;

  void setTwistAngle(double angle);
  double twistAngle() const;

  // Extrusion
  bool canExtrude() const;
  bool performExtrusion();

private:
  QWidget* doCreatePage(QWidget* parent) override;
};

} // namespace ui
} // namespace tb
