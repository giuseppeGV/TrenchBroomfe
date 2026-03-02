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

#include "vm/vec.h"

#include <vector>

class QDoubleSpinBox;
class QListWidget;
class QPushButton;
class QSpinBox;

namespace tb
{
namespace mdl
{
class Map;
}

namespace ui
{
class MapDocument;

class CurvePathDialog : public QDialog
{
  Q_OBJECT
private:
  MapDocument& m_document;

  QListWidget* m_pointsList = nullptr;
  QPushButton* m_addPointButton = nullptr;
  QPushButton* m_removePointButton = nullptr;
  QPushButton* m_fromSelectionButton = nullptr;

  QDoubleSpinBox* m_widthSpin = nullptr;
  QDoubleSpinBox* m_heightSpin = nullptr;
  QSpinBox* m_segmentsSpin = nullptr;

  std::vector<vm::vec3d> m_controlPoints;

public:
  explicit CurvePathDialog(MapDocument& document, QWidget* parent = nullptr);

private:
  void createGui();
  void accept() override;

  void addPoint();
  void removeSelectedPoint();
  void loadFromSelection();
  void refreshPointsList();

  static std::vector<vm::vec3d> evaluateCatmullRom(
    const std::vector<vm::vec3d>& controlPoints, size_t segmentsPerSpan);
};

} // namespace ui
} // namespace tb
