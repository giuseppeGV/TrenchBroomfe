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

#include "ui/MeasureTool.h"

class QLabel;
class QPushButton;

namespace tb::ui
{
class MapDocument;
class SpinControl;

/**
 * UI page for the Measure Tool, displaying measurement information.
 */
class MeasureToolPage : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;
  MeasureTool& m_tool;

  // Selection info labels
  QLabel* m_selectionDimX = nullptr;
  QLabel* m_selectionDimY = nullptr;
  QLabel* m_selectionDimZ = nullptr;
  QLabel* m_selectionDiagonal = nullptr;
  
  // Object counts
  QLabel* m_brushCount = nullptr;
  QLabel* m_entityCount = nullptr;
  QLabel* m_faceCount = nullptr;
  QLabel* m_vertexCount = nullptr;

  // Calculated values
  QLabel* m_totalArea = nullptr;
  QLabel* m_totalVolume = nullptr;

  // Point-to-point measurement
  SpinControl* m_startX = nullptr;
  SpinControl* m_startY = nullptr;
  SpinControl* m_startZ = nullptr;
  SpinControl* m_endX = nullptr;
  SpinControl* m_endY = nullptr;
  SpinControl* m_endZ = nullptr;
  QLabel* m_pointDistance = nullptr;
  QLabel* m_pointDistX = nullptr;
  QLabel* m_pointDistY = nullptr;
  QLabel* m_pointDistZ = nullptr;

  QPushButton* m_refreshButton = nullptr;
  QPushButton* m_useSelectionStartButton = nullptr;
  QPushButton* m_useSelectionEndButton = nullptr;

public:
  MeasureToolPage(MapDocument& document, MeasureTool& tool, QWidget* parent = nullptr);

private:
  void createGui();
  void updateMeasurements();

  void refreshClicked();
  void useSelectionStartClicked();
  void useSelectionEndClicked();
  void pointChanged();
};

} // namespace tb::ui
