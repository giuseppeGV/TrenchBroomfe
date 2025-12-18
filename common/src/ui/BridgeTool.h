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
class BrushFaceHandle;
}

namespace ui
{
class MapDocument;

/**
 * Tool for creating a bridge (connecting geometry) between two selected brush faces.
 */
class BridgeTool : public Tool
{
private:
  MapDocument& m_document;
  
  // Bridge settings
  int m_segments = 1;
  bool m_curvedBridge = false;
  double m_curvature = 0.0;
  bool m_taper = false;
  double m_taperAmount = 0.0;

public:
  explicit BridgeTool(MapDocument& document);

  bool doActivate() override;

  /**
   * Sets the number of segments for the bridge
   */
  void setSegments(int segments);
  int segments() const;

  /**
   * Enable/disable curved bridge
   */
  void setCurved(bool curved);
  bool isCurved() const;

  /**
   * Set curvature amount (-1.0 to 1.0)
   */
  void setCurvature(double curvature);
  double curvature() const;

  /**
   * Enable/disable tapering
   */
  void setTaper(bool taper);
  bool isTapered() const;

  /**
   * Set taper amount
   */
  void setTaperAmount(double amount);
  double taperAmount() const;

  /**
   * Creates a bridge between two selected faces.
   * @return true if the bridge was created successfully
   */
  bool createBridge();

  /**
   * Checks if a bridge can be created with the current selection.
   * Requires exactly two brush faces to be selected.
   */
  bool canCreateBridge() const;

private:
  QWidget* doCreatePage(QWidget* parent) override;
  
  /**
   * Gets the two selected faces for bridging
   */
  std::optional<std::pair<mdl::BrushFaceHandle, mdl::BrushFaceHandle>> getSelectedFaces() const;
};

} // namespace ui
} // namespace tb
