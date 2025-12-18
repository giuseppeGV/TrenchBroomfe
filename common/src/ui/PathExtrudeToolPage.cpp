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

#include "PathExtrudeToolPage.h"
#include "PathExtrudeTool.h"

#include "mdl/Map.h"
#include "mdl/Selection.h"
#include "ui/MapDocument.h"
#include "ui/SpinControl.h"
#include "ui/ViewConstants.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace tb::ui
{

PathExtrudeToolPage::PathExtrudeToolPage(MapDocument& document, PathExtrudeTool& tool, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
  , m_tool{tool}
{
  createGui();
  updateGui();
}

void PathExtrudeToolPage::createGui()
{
  auto* mainLayout = new QVBoxLayout{};
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(LayoutConstants::MediumVMargin);

  // Instructions
  m_statusLabel = new QLabel{tr("Select brushes and add path waypoints to extrude along.")};
  m_statusLabel->setWordWrap(true);
  mainLayout->addWidget(m_statusLabel);

  // Waypoint group
  auto* waypointGroup = new QGroupBox{tr("Path Waypoints")};
  auto* waypointLayout = new QVBoxLayout{};

  m_waypointList = new QListWidget{};
  m_waypointList->setMaximumHeight(100);
  waypointLayout->addWidget(m_waypointList);

  auto* pointButtonLayout = new QHBoxLayout{};
  m_addPointButton = new QPushButton{tr("Add Point...")};
  m_removePointButton = new QPushButton{tr("Remove")};
  m_clearPointsButton = new QPushButton{tr("Clear")};
  
  connect(m_addPointButton, &QPushButton::clicked, this, &PathExtrudeToolPage::addPointClicked);
  connect(m_removePointButton, &QPushButton::clicked, this, &PathExtrudeToolPage::removePointClicked);
  connect(m_clearPointsButton, &QPushButton::clicked, this, &PathExtrudeToolPage::clearPointsClicked);
  
  pointButtonLayout->addWidget(m_addPointButton);
  pointButtonLayout->addWidget(m_removePointButton);
  pointButtonLayout->addWidget(m_clearPointsButton);
  waypointLayout->addLayout(pointButtonLayout);

  waypointGroup->setLayout(waypointLayout);
  mainLayout->addWidget(waypointGroup);

  // Settings group
  auto* settingsGroup = new QGroupBox{tr("Extrusion Settings")};
  auto* settingsLayout = new QGridLayout{};

  int row = 0;

  // Segments per waypoint
  settingsLayout->addWidget(new QLabel{tr("Segments:")}, row, 0);
  m_segmentsSpinner = new QSpinBox{};
  m_segmentsSpinner->setRange(1, 32);
  m_segmentsSpinner->setValue(m_tool.segments());
  connect(m_segmentsSpinner, QOverload<int>::of(&QSpinBox::valueChanged),
          this, &PathExtrudeToolPage::segmentsChanged);
  settingsLayout->addWidget(m_segmentsSpinner, row, 1);
  row++;

  // Align to path
  m_alignCheck = new QCheckBox{tr("Align to path direction")};
  m_alignCheck->setChecked(m_tool.alignToPath());
  connect(m_alignCheck, &QCheckBox::toggled, this, &PathExtrudeToolPage::alignChanged);
  settingsLayout->addWidget(m_alignCheck, row, 0, 1, 2);
  row++;

  // Scale along path
  m_scaleCheck = new QCheckBox{tr("Scale along path")};
  m_scaleCheck->setChecked(m_tool.scaleAlongPath());
  connect(m_scaleCheck, &QCheckBox::toggled, this, &PathExtrudeToolPage::scaleChanged);
  settingsLayout->addWidget(m_scaleCheck, row, 0, 1, 2);
  row++;

  // Start scale
  settingsLayout->addWidget(new QLabel{tr("Start Scale:")}, row, 0);
  m_startScaleSpin = new SpinControl{this};
  m_startScaleSpin->setRange(0.01, 10.0);
  m_startScaleSpin->setValue(m_tool.startScale());
  m_startScaleSpin->setEnabled(m_tool.scaleAlongPath());
  connect(m_startScaleSpin, &SpinControl::valueChanged, this, &PathExtrudeToolPage::startScaleChanged);
  settingsLayout->addWidget(m_startScaleSpin, row, 1);
  row++;

  // End scale
  settingsLayout->addWidget(new QLabel{tr("End Scale:")}, row, 0);
  m_endScaleSpin = new SpinControl{this};
  m_endScaleSpin->setRange(0.01, 10.0);
  m_endScaleSpin->setValue(m_tool.endScale());
  m_endScaleSpin->setEnabled(m_tool.scaleAlongPath());
  connect(m_endScaleSpin, &SpinControl::valueChanged, this, &PathExtrudeToolPage::endScaleChanged);
  settingsLayout->addWidget(m_endScaleSpin, row, 1);
  row++;

  // Twist
  m_twistCheck = new QCheckBox{tr("Twist along path")};
  m_twistCheck->setChecked(m_tool.twist());
  connect(m_twistCheck, &QCheckBox::toggled, this, &PathExtrudeToolPage::twistChanged);
  settingsLayout->addWidget(m_twistCheck, row, 0, 1, 2);
  row++;

  // Twist angle
  settingsLayout->addWidget(new QLabel{tr("Twist Angle (°/seg):")}, row, 0);
  m_twistAngleSpin = new SpinControl{this};
  m_twistAngleSpin->setRange(-180.0, 180.0);
  m_twistAngleSpin->setValue(m_tool.twistAngle());
  m_twistAngleSpin->setEnabled(m_tool.twist());
  connect(m_twistAngleSpin, &SpinControl::valueChanged, this, &PathExtrudeToolPage::twistAngleChanged);
  settingsLayout->addWidget(m_twistAngleSpin, row, 1);
  row++;

  settingsGroup->setLayout(settingsLayout);
  mainLayout->addWidget(settingsGroup);

  // Extrude button
  m_extrudeButton = new QPushButton{tr("Extrude Along Path")};
  m_extrudeButton->setDefault(true);
  connect(m_extrudeButton, &QPushButton::clicked, this, &PathExtrudeToolPage::extrudeClicked);
  mainLayout->addWidget(m_extrudeButton);

  mainLayout->addStretch(1);

  setLayout(mainLayout);
}

void PathExtrudeToolPage::updateGui()
{
  const bool canExtrude = m_tool.canExtrude();
  m_extrudeButton->setEnabled(canExtrude);
  m_removePointButton->setEnabled(m_tool.waypointCount() > 0);
  m_clearPointsButton->setEnabled(m_tool.waypointCount() > 0);
  
  if (canExtrude)
  {
    m_statusLabel->setText(tr("Ready to extrude along %1 waypoints.").arg(m_tool.waypointCount()));
    m_statusLabel->setStyleSheet("color: #a6e3a1;"); // Green
  }
  else
  {
    const auto& map = m_document.map();
    const bool hasBrushes = map.selection().hasOnlyBrushes();
    const size_t points = m_tool.waypointCount();
    
    if (!hasBrushes)
    {
      m_statusLabel->setText(tr("Select one or more brushes to extrude."));
    }
    else if (points < 2)
    {
      m_statusLabel->setText(tr("Add at least 2 waypoints. (%1 of 2)").arg(points));
    }
    m_statusLabel->setStyleSheet("color: #f9e2af;"); // Yellow
  }
  
  updateWaypointList();
}

void PathExtrudeToolPage::updateWaypointList()
{
  m_waypointList->clear();
  const auto& waypoints = m_tool.waypoints();
  for (size_t i = 0; i < waypoints.size(); ++i)
  {
    const auto& p = waypoints[i];
    m_waypointList->addItem(QString("Point %1: (%2, %3, %4)")
      .arg(i + 1)
      .arg(p.x(), 0, 'f', 1)
      .arg(p.y(), 0, 'f', 1)
      .arg(p.z(), 0, 'f', 1));
  }
}

void PathExtrudeToolPage::addPointClicked()
{
  bool ok;
  const QString text = QInputDialog::getText(this, tr("Add Waypoint"),
    tr("Enter coordinates (X Y Z):"), QLineEdit::Normal, "0 0 0", &ok);
  
  if (ok && !text.isEmpty())
  {
    const auto parts = text.split(' ', Qt::SkipEmptyParts);
    if (parts.size() >= 3)
    {
      const double x = parts[0].toDouble();
      const double y = parts[1].toDouble();
      const double z = parts[2].toDouble();
      m_tool.addWaypoint(vm::vec3d{x, y, z});
      updateGui();
    }
  }
}

void PathExtrudeToolPage::removePointClicked()
{
  m_tool.removeLastWaypoint();
  updateGui();
}

void PathExtrudeToolPage::clearPointsClicked()
{
  m_tool.clearWaypoints();
  updateGui();
}

void PathExtrudeToolPage::segmentsChanged(int value)
{
  m_tool.setSegments(value);
}

void PathExtrudeToolPage::alignChanged(bool checked)
{
  m_tool.setAlignToPath(checked);
}

void PathExtrudeToolPage::scaleChanged(bool checked)
{
  m_tool.setScaleAlongPath(checked);
  m_startScaleSpin->setEnabled(checked);
  m_endScaleSpin->setEnabled(checked);
}

void PathExtrudeToolPage::startScaleChanged(double value)
{
  m_tool.setStartScale(value);
}

void PathExtrudeToolPage::endScaleChanged(double value)
{
  m_tool.setEndScale(value);
}

void PathExtrudeToolPage::twistChanged(bool checked)
{
  m_tool.setTwist(checked);
  m_twistAngleSpin->setEnabled(checked);
}

void PathExtrudeToolPage::twistAngleChanged(double value)
{
  m_tool.setTwistAngle(value);
}

void PathExtrudeToolPage::extrudeClicked()
{
  if (m_tool.performExtrusion())
  {
    updateGui();
  }
}

} // namespace tb::ui
