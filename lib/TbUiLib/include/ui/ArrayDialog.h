/*
 Copyright (C) 2024 Kristian Duske

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

#include <QDialog>

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QSpinBox;
class QStackedWidget;

namespace tb
{
namespace mdl
{
class Map;
}

namespace ui
{
class MapDocument;

class ArrayDialog : public QDialog
{
  Q_OBJECT
private:
  MapDocument& m_document;

  QComboBox* m_modeCombo = nullptr;
  QSpinBox* m_countSpin = nullptr;
  QStackedWidget* m_modeStack = nullptr;

  // Linear mode widgets
  QDoubleSpinBox* m_offsetX = nullptr;
  QDoubleSpinBox* m_offsetY = nullptr;
  QDoubleSpinBox* m_offsetZ = nullptr;

  // Radial mode widgets
  QDoubleSpinBox* m_centerX = nullptr;
  QDoubleSpinBox* m_centerY = nullptr;
  QDoubleSpinBox* m_centerZ = nullptr;
  QComboBox* m_axisCombo = nullptr;
  QDoubleSpinBox* m_totalAngle = nullptr;

public:
  explicit ArrayDialog(MapDocument& document, QWidget* parent = nullptr);

private:
  void createGui();
  void accept() override;
private slots:
  void modeChanged(int index);
};

} // namespace ui
} // namespace tb
