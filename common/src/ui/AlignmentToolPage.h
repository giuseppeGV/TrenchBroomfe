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

#include "ui/AlignmentTool.h"

class QPushButton;
class QComboBox;
class QCheckBox;

namespace tb::ui
{
class MapDocument;
class SpinControl;

/**
 * UI page for the Alignment Tool, providing controls for aligning and distributing objects.
 */
class AlignmentToolPage : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;
  AlignmentTool& m_tool;

  // Align controls
  QComboBox* m_alignAxis = nullptr;
  QComboBox* m_alignMode = nullptr;
  QCheckBox* m_alignToFirst = nullptr;
  QPushButton* m_alignButton = nullptr;

  // Distribute controls
  QComboBox* m_distributeAxis = nullptr;
  QCheckBox* m_useSpacing = nullptr;
  SpinControl* m_spacing = nullptr;
  QPushButton* m_distributeButton = nullptr;

  // Align to grid controls
  QComboBox* m_gridAxis = nullptr;
  QComboBox* m_gridMode = nullptr;
  QPushButton* m_gridButton = nullptr;

  // Stack controls
  QComboBox* m_stackAxis = nullptr;
  SpinControl* m_stackGap = nullptr;
  QPushButton* m_stackButton = nullptr;

public:
  AlignmentToolPage(MapDocument& document, AlignmentTool& tool, QWidget* parent = nullptr);

private:
  void createGui();
  void updateGui();

  void alignClicked();
  void distributeClicked();
  void alignToGridClicked();
  void stackClicked();
};

} // namespace tb::ui
