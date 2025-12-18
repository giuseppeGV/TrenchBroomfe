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

#include "ui/BridgeTool.h"

class QPushButton;
class QSpinBox;
class QCheckBox;
class QLabel;

namespace tb::ui
{
class MapDocument;
class SpinControl;

/**
 * UI page for the Bridge Tool
 */
class BridgeToolPage : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;
  BridgeTool& m_tool;

  QSpinBox* m_segmentsSpinner = nullptr;
  QCheckBox* m_curvedCheck = nullptr;
  SpinControl* m_curvatureSpin = nullptr;
  QCheckBox* m_taperCheck = nullptr;
  SpinControl* m_taperAmountSpin = nullptr;
  QLabel* m_statusLabel = nullptr;
  QPushButton* m_createButton = nullptr;

public:
  BridgeToolPage(MapDocument& document, BridgeTool& tool, QWidget* parent = nullptr);

private:
  void createGui();
  void updateGui();

  void segmentsChanged(int value);
  void curvedChanged(bool checked);
  void curvatureChanged(double value);
  void taperChanged(bool checked);
  void taperAmountChanged(double value);
  void createBridgeClicked();
};

} // namespace tb::ui
