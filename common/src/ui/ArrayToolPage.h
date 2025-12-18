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

#include <QWidget>

#include "ui/ArrayTool.h"

class QPushButton;
class QComboBox;
class QCheckBox;
class QStackedWidget;
class QSpinBox;

namespace tb::ui
{
class MapDocument;
class SpinControl;

/**
 * UI page for the Array Tool, providing controls for linear, grid, and radial arrays.
 */
class ArrayToolPage : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;
  ArrayTool& m_tool;

  // Mode selection
  QComboBox* m_modeCombo = nullptr;
  QStackedWidget* m_stackedWidget = nullptr;

  // Linear array controls
  QSpinBox* m_linearCount = nullptr;
  SpinControl* m_linearOffsetX = nullptr;
  SpinControl* m_linearOffsetY = nullptr;
  SpinControl* m_linearOffsetZ = nullptr;

  // Grid array controls
  QSpinBox* m_gridCountX = nullptr;
  QSpinBox* m_gridCountY = nullptr;
  QSpinBox* m_gridCountZ = nullptr;
  SpinControl* m_gridSpacingX = nullptr;
  SpinControl* m_gridSpacingY = nullptr;
  SpinControl* m_gridSpacingZ = nullptr;

  // Radial array controls
  QSpinBox* m_radialCount = nullptr;
  SpinControl* m_radialCenterX = nullptr;
  SpinControl* m_radialCenterY = nullptr;
  SpinControl* m_radialCenterZ = nullptr;
  QComboBox* m_radialAxis = nullptr;
  SpinControl* m_radialAngle = nullptr;

  // Common controls
  QCheckBox* m_groupResult = nullptr;
  QPushButton* m_applyButton = nullptr;

public:
  ArrayToolPage(MapDocument& document, ArrayTool& tool, QWidget* parent = nullptr);

private:
  void createGui();
  QWidget* createLinearPage();
  QWidget* createGridPage();
  QWidget* createRadialPage();
  void updateGui();

  void modeChanged(int index);
  void applyClicked();
  void useSelectionCenterClicked();
};

} // namespace tb::ui
