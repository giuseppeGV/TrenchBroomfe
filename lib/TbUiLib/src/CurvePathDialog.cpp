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

#include "ui/CurvePathDialog.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Node.h"
#include "mdl/Transaction.h"
#include "ui/MapDocument.h"
#include "ui/QVecUtils.h"

#include "vm/bbox.h"
#include "vm/vec.h"

#include <fmt/format.h>

namespace tb::ui
{

CurvePathDialog::CurvePathDialog(MapDocument& document, QWidget* parent)
  : QDialog{parent}
  , m_document{document}
{
  createGui();
}

void CurvePathDialog::createGui()
{
  setWindowIconTB(this);
  setWindowTitle(tr("Curve / Path"));
  setMinimumWidth(400);

  // Control points list
  m_pointsList = new QListWidget{};
  m_pointsList->setMinimumHeight(120);

  m_addPointButton = new QPushButton{tr("Add Point...")};
  m_removePointButton = new QPushButton{tr("Remove")};
  m_fromSelectionButton = new QPushButton{tr("From Selection")};

  connect(m_addPointButton, &QPushButton::clicked, this, &CurvePathDialog::addPoint);
  connect(
    m_removePointButton,
    &QPushButton::clicked,
    this,
    &CurvePathDialog::removeSelectedPoint);
  connect(
    m_fromSelectionButton,
    &QPushButton::clicked,
    this,
    &CurvePathDialog::loadFromSelection);

  auto* pointButtonLayout = new QHBoxLayout{};
  pointButtonLayout->addWidget(m_addPointButton);
  pointButtonLayout->addWidget(m_removePointButton);
  pointButtonLayout->addWidget(m_fromSelectionButton);

  // Cross-section settings
  m_widthSpin = new QDoubleSpinBox{};
  m_widthSpin->setRange(1.0, 4096.0);
  m_widthSpin->setValue(32.0);
  m_widthSpin->setDecimals(1);

  m_heightSpin = new QDoubleSpinBox{};
  m_heightSpin->setRange(1.0, 4096.0);
  m_heightSpin->setValue(32.0);
  m_heightSpin->setDecimals(1);

  m_segmentsSpin = new QSpinBox{};
  m_segmentsSpin->setRange(1, 64);
  m_segmentsSpin->setValue(8);

  auto* sectionForm = new QFormLayout{};
  sectionForm->addRow(tr("Width:"), m_widthSpin);
  sectionForm->addRow(tr("Height:"), m_heightSpin);
  sectionForm->addRow(tr("Segments per span:"), m_segmentsSpin);

  // Buttons
  auto* buttonBox = new QDialogButtonBox{};
  buttonBox->addButton(tr("Create Curve"), QDialogButtonBox::AcceptRole);
  buttonBox->addButton(QDialogButtonBox::Cancel);

  connect(buttonBox, &QDialogButtonBox::accepted, this, &CurvePathDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  // Main layout
  auto* layout = new QVBoxLayout{};
  layout->addWidget(new QLabel{tr("Control Points (minimum 2):")});
  layout->addWidget(m_pointsList);
  layout->addLayout(pointButtonLayout);
  layout->addSpacing(8);
  layout->addWidget(new QLabel{tr("Cross-Section:")});
  layout->addLayout(sectionForm);
  layout->addWidget(buttonBox);
  setLayout(layout);
}

void CurvePathDialog::addPoint()
{
  bool ok = false;
  const auto text = QInputDialog::getText(
    this,
    tr("Add Control Point"),
    tr("Enter position (x y z):"),
    QLineEdit::Normal,
    "0 0 0",
    &ok);

  if (ok && !text.isEmpty())
  {
    const auto parts = text.split(' ', Qt::SkipEmptyParts);
    if (parts.size() >= 3)
    {
      const auto x = parts[0].toDouble();
      const auto y = parts[1].toDouble();
      const auto z = parts[2].toDouble();
      m_controlPoints.push_back(vm::vec3d{x, y, z});
      refreshPointsList();
    }
    else
    {
      QMessageBox::warning(
        this, tr("Invalid Input"), tr("Please enter three numbers separated by spaces."));
    }
  }
}

void CurvePathDialog::removeSelectedPoint()
{
  const auto row = m_pointsList->currentRow();
  if (row >= 0 && row < static_cast<int>(m_controlPoints.size()))
  {
    m_controlPoints.erase(m_controlPoints.begin() + row);
    refreshPointsList();
  }
}

void CurvePathDialog::loadFromSelection()
{
  const auto& map = m_document.map();
  const auto& selection = map.selection();

  m_controlPoints.clear();
  for (const auto* node : selection.nodes)
  {
    const auto bounds = node->logicalBounds();
    m_controlPoints.push_back(bounds.center());
  }

  if (m_controlPoints.empty())
  {
    QMessageBox::information(
      this,
      tr("No Selection"),
      tr("Select some objects to use their centers as control points."));
  }
  refreshPointsList();
}

void CurvePathDialog::refreshPointsList()
{
  m_pointsList->clear();
  for (size_t i = 0; i < m_controlPoints.size(); ++i)
  {
    const auto& p = m_controlPoints[i];
    m_pointsList->addItem(
      QString::fromStdString(
        fmt::format("{}: ({:.1f}, {:.1f}, {:.1f})", i + 1, p.x(), p.y(), p.z())));
  }
}

std::vector<vm::vec3d> CurvePathDialog::evaluateCatmullRom(
  const std::vector<vm::vec3d>& controlPoints, const size_t segmentsPerSpan)
{
  if (controlPoints.size() < 2)
  {
    return controlPoints;
  }

  std::vector<vm::vec3d> result;

  // For Catmull-Rom, we need 4 points per segment: P0, P1, P2, P3
  // We duplicate the first and last points to handle ends
  std::vector<vm::vec3d> pts;
  pts.push_back(controlPoints.front()); // duplicate first
  for (const auto& p : controlPoints)
  {
    pts.push_back(p);
  }
  pts.push_back(controlPoints.back()); // duplicate last

  for (size_t i = 1; i + 2 < pts.size(); ++i)
  {
    const auto& p0 = pts[i - 1];
    const auto& p1 = pts[i];
    const auto& p2 = pts[i + 1];
    const auto& p3 = pts[i + 2];

    for (size_t j = 0; j < segmentsPerSpan; ++j)
    {
      const auto t = static_cast<double>(j) / static_cast<double>(segmentsPerSpan);
      const auto t2 = t * t;
      const auto t3 = t2 * t;

      // Catmull-Rom spline formula
      const auto point =
        0.5
        * ((2.0 * p1) + (-p0 + p2) * t + (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) * t2 + (-p0 + 3.0 * p1 - 3.0 * p2 + p3) * t3);
      result.push_back(point);
    }
  }

  // Add the last point
  result.push_back(controlPoints.back());

  return result;
}

void CurvePathDialog::accept()
{
  if (m_controlPoints.size() < 2)
  {
    QMessageBox::warning(
      this, tr("Not Enough Points"), tr("At least 2 control points are required."));
    return;
  }

  const auto segmentsPerSpan = static_cast<size_t>(m_segmentsSpin->value());
  const auto curvePoints = evaluateCatmullRom(m_controlPoints, segmentsPerSpan);

  if (curvePoints.size() < 2)
  {
    QMessageBox::warning(this, tr("Error"), tr("Failed to generate curve points."));
    return;
  }

  const auto halfWidth = m_widthSpin->value() / 2.0;
  const auto halfHeight = m_heightSpin->value() / 2.0;

  auto& map = m_document.map();
  const auto defaultMaterialName = map.defaultMaterialName();
  const auto mapFormat = map.mapFormat();
  const auto worldBounds = map.worldBounds();

  const mdl::BrushBuilder builder{mapFormat, worldBounds};

  // Build brushes along the curve
  std::vector<mdl::Node*> newNodes;
  for (size_t i = 0; i + 1 < curvePoints.size(); ++i)
  {
    const auto& start = curvePoints[i];
    const auto& end = curvePoints[i + 1];
    const auto dir = end - start;
    const auto len = vm::length(dir);

    if (len < 0.001)
    {
      continue;
    }

    const auto forward = dir / len;

    // Compute a stable up vector
    auto up = vm::vec3d{0, 0, 1};
    if (std::abs(vm::dot(forward, up)) > 0.99)
    {
      up = vm::vec3d{0, 1, 0};
    }

    const auto right = vm::normalize(vm::cross(forward, up));
    const auto realUp = vm::cross(right, forward);

    // Four corners at start
    const auto s0 = start - right * halfWidth - realUp * halfHeight;
    const auto s1 = start + right * halfWidth - realUp * halfHeight;
    const auto s2 = start + right * halfWidth + realUp * halfHeight;
    const auto s3 = start - right * halfWidth + realUp * halfHeight;

    // Four corners at end
    const auto e0 = end - right * halfWidth - realUp * halfHeight;
    const auto e1 = end + right * halfWidth - realUp * halfHeight;
    const auto e2 = end + right * halfWidth + realUp * halfHeight;
    const auto e3 = end - right * halfWidth + realUp * halfHeight;

    const std::vector<vm::vec3d> vertices = {s0, s1, s2, s3, e0, e1, e2, e3};

    auto brushResult = builder.createBrush(vertices, defaultMaterialName);
    if (brushResult.is_success())
    {
      newNodes.push_back(new mdl::BrushNode{std::move(brushResult).value()});
    }
  }

  if (newNodes.empty())
  {
    QMessageBox::warning(
      this, tr("Error"), tr("No brushes could be created from the curve."));
    return;
  }

  auto* parentNode = mdl::parentForNodes(map);
  auto transaction = mdl::Transaction{map, "Create Curve Path"};
  mdl::deselectAll(map);

  const auto added = mdl::addNodes(map, {{parentNode, std::move(newNodes)}});
  if (added.empty())
  {
    transaction.cancel();
    QMessageBox::warning(
      this, tr("Error"), tr("Failed to add curve brushes to the map."));
    return;
  }

  mdl::selectNodes(map, added);
  transaction.commit();

  QDialog::accept();
}

} // namespace tb::ui
