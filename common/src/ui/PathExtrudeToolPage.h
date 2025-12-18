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

class QPushButton;
class QSpinBox;
class QCheckBox;
class QLabel;
class QListWidget;

namespace tb::ui
{
class MapDocument;
class PathExtrudeTool;
class SpinControl;

/**
 * UI page for the Path Extrude Tool
 */
class PathExtrudeToolPage : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;
  PathExtrudeTool& m_tool;

  QListWidget* m_waypointList = nullptr;
  QPushButton* m_addPointButton = nullptr;
  QPushButton* m_removePointButton = nullptr;
  QPushButton* m_clearPointsButton = nullptr;
  
  QSpinBox* m_segmentsSpinner = nullptr;
  QCheckBox* m_alignCheck = nullptr;
  QCheckBox* m_scaleCheck = nullptr;
  SpinControl* m_startScaleSpin = nullptr;
  SpinControl* m_endScaleSpin = nullptr;
  QCheckBox* m_twistCheck = nullptr;
  SpinControl* m_twistAngleSpin = nullptr;
  
  QLabel* m_statusLabel = nullptr;
  QPushButton* m_extrudeButton = nullptr;

public:
  PathExtrudeToolPage(MapDocument& document, PathExtrudeTool& tool, QWidget* parent = nullptr);

private:
  void createGui();
  void updateGui();
  void updateWaypointList();

  void addPointClicked();
  void removePointClicked();
  void clearPointsClicked();
  void segmentsChanged(int value);
  void alignChanged(bool checked);
  void scaleChanged(bool checked);
  void startScaleChanged(double value);
  void endScaleChanged(double value);
  void twistChanged(bool checked);
  void twistAngleChanged(double value);
  void extrudeClicked();
};

} // namespace tb::ui
