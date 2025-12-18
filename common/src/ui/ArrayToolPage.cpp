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

#include "ArrayToolPage.h"
#include "ArrayTool.h"

#include "mdl/Map.h"
#include "mdl/Selection.h"
#include "ui/MapDocument.h"
#include "ui/SpinControl.h"
#include "ui/ViewConstants.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace tb::ui
{

ArrayToolPage::ArrayToolPage(MapDocument& document, ArrayTool& tool, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
  , m_tool{tool}
{
  createGui();
  updateGui();
}

void ArrayToolPage::createGui()
{
  auto* mainLayout = new QVBoxLayout{};
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(LayoutConstants::MediumVMargin);

  // Mode selector
  auto* modeLayout = new QHBoxLayout{};
  modeLayout->addWidget(new QLabel{tr("Array Mode:")});
  m_modeCombo = new QComboBox{};
  m_modeCombo->addItem(tr("Linear"));
  m_modeCombo->addItem(tr("Grid"));
  m_modeCombo->addItem(tr("Radial"));
  modeLayout->addWidget(m_modeCombo, 1);
  mainLayout->addLayout(modeLayout);

  // Stacked widget for different modes
  m_stackedWidget = new QStackedWidget{};
  m_stackedWidget->addWidget(createLinearPage());
  m_stackedWidget->addWidget(createGridPage());
  m_stackedWidget->addWidget(createRadialPage());
  mainLayout->addWidget(m_stackedWidget);

  // Common controls
  m_groupResult = new QCheckBox{tr("Group resulting objects")};
  mainLayout->addWidget(m_groupResult);

  // Apply button
  m_applyButton = new QPushButton{tr("Create Array")};
  m_applyButton->setDefault(true);
  mainLayout->addWidget(m_applyButton);

  mainLayout->addStretch(1);

  setLayout(mainLayout);

  // Connect signals
  connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &ArrayToolPage::modeChanged);
  connect(m_applyButton, &QPushButton::clicked,
          this, &ArrayToolPage::applyClicked);
}

QWidget* ArrayToolPage::createLinearPage()
{
  auto* page = new QWidget{};
  auto* layout = new QGridLayout{};
  layout->setContentsMargins(0, 0, 0, 0);

  int row = 0;

  // Count
  layout->addWidget(new QLabel{tr("Number of Copies:")}, row, 0);
  m_linearCount = new QSpinBox{};
  m_linearCount->setRange(1, 1000);
  m_linearCount->setValue(3);
  layout->addWidget(m_linearCount, row, 1, 1, 3);
  row++;

  // Offset
  layout->addWidget(new QLabel{tr("Offset X/Y/Z:")}, row, 0);
  
  auto createDoubleSpin = [this](double val) {
    auto* spin = new SpinControl{this};
    spin->setRange(-10000.0, 10000.0);
    spin->setValue(val);
    return spin;
  };

  m_linearOffsetX = createDoubleSpin(64.0);
  m_linearOffsetY = createDoubleSpin(0.0);
  m_linearOffsetZ = createDoubleSpin(0.0);
  layout->addWidget(m_linearOffsetX, row, 1);
  layout->addWidget(m_linearOffsetY, row, 2);
  layout->addWidget(m_linearOffsetZ, row, 3);
  row++;

  layout->setColumnStretch(1, 1);
  layout->setColumnStretch(2, 1);
  layout->setColumnStretch(3, 1);
  layout->setRowStretch(row, 1);

  page->setLayout(layout);
  return page;
}

QWidget* ArrayToolPage::createGridPage()
{
  auto* page = new QWidget{};
  auto* layout = new QGridLayout{};
  layout->setContentsMargins(0, 0, 0, 0);

  int row = 0;

  // Count
  layout->addWidget(new QLabel{tr("Count X/Y/Z:")}, row, 0);
  m_gridCountX = new QSpinBox{};
  m_gridCountX->setRange(1, 100);
  m_gridCountX->setValue(3);
  m_gridCountY = new QSpinBox{};
  m_gridCountY->setRange(1, 100);
  m_gridCountY->setValue(3);
  m_gridCountZ = new QSpinBox{};
  m_gridCountZ->setRange(1, 100);
  m_gridCountZ->setValue(1);
  layout->addWidget(m_gridCountX, row, 1);
  layout->addWidget(m_gridCountY, row, 2);
  layout->addWidget(m_gridCountZ, row, 3);
  row++;

  // Spacing
  layout->addWidget(new QLabel{tr("Spacing X/Y/Z:")}, row, 0);
  
  auto createDoubleSpin = [this](double val) {
    auto* spin = new SpinControl{this};
    spin->setRange(-10000.0, 10000.0);
    spin->setValue(val);
    return spin;
  };

  m_gridSpacingX = createDoubleSpin(8.0);
  m_gridSpacingY = createDoubleSpin(8.0);
  m_gridSpacingZ = createDoubleSpin(8.0);
  layout->addWidget(m_gridSpacingX, row, 1);
  layout->addWidget(m_gridSpacingY, row, 2);
  layout->addWidget(m_gridSpacingZ, row, 3);
  row++;

  layout->setColumnStretch(1, 1);
  layout->setColumnStretch(2, 1);
  layout->setColumnStretch(3, 1);
  layout->setRowStretch(row, 1);

  page->setLayout(layout);
  return page;
}

QWidget* ArrayToolPage::createRadialPage()
{
  auto* page = new QWidget{};
  auto* layout = new QGridLayout{};
  layout->setContentsMargins(0, 0, 0, 0);

  int row = 0;

  // Count
  layout->addWidget(new QLabel{tr("Number of Copies:")}, row, 0);
  m_radialCount = new QSpinBox{};
  m_radialCount->setRange(1, 360);
  m_radialCount->setValue(8);
  layout->addWidget(m_radialCount, row, 1, 1, 3);
  row++;

  // Center point
  layout->addWidget(new QLabel{tr("Center X/Y/Z:")}, row, 0);
  
  auto createDoubleSpin = [this](double val) {
    auto* spin = new SpinControl{this};
    spin->setRange(-100000.0, 100000.0);
    spin->setValue(val);
    return spin;
  };

  m_radialCenterX = createDoubleSpin(0.0);
  m_radialCenterY = createDoubleSpin(0.0);
  m_radialCenterZ = createDoubleSpin(0.0);
  layout->addWidget(m_radialCenterX, row, 1);
  layout->addWidget(m_radialCenterY, row, 2);
  layout->addWidget(m_radialCenterZ, row, 3);
  row++;

  // Use selection center button
  auto* useCenterBtn = new QPushButton{tr("Use Selection Center")};
  connect(useCenterBtn, &QPushButton::clicked, this, &ArrayToolPage::useSelectionCenterClicked);
  layout->addWidget(useCenterBtn, row, 1, 1, 3);
  row++;

  // Axis
  layout->addWidget(new QLabel{tr("Rotation Axis:")}, row, 0);
  m_radialAxis = new QComboBox{};
  m_radialAxis->addItem(tr("X Axis"));
  m_radialAxis->addItem(tr("Y Axis"));
  m_radialAxis->addItem(tr("Z Axis"));
  m_radialAxis->setCurrentIndex(2); // Z axis default
  layout->addWidget(m_radialAxis, row, 1, 1, 3);
  row++;

  // Total angle
  layout->addWidget(new QLabel{tr("Total Angle (degrees):")}, row, 0);
  m_radialAngle = createDoubleSpin(360.0);
  m_radialAngle->setRange(0.0, 360.0);
  layout->addWidget(m_radialAngle, row, 1, 1, 3);
  row++;

  layout->setColumnStretch(1, 1);
  layout->setColumnStretch(2, 1);
  layout->setColumnStretch(3, 1);
  layout->setRowStretch(row, 1);

  page->setLayout(layout);
  return page;
}

void ArrayToolPage::updateGui()
{
}

void ArrayToolPage::modeChanged(int index)
{
  m_stackedWidget->setCurrentIndex(index);
}

void ArrayToolPage::applyClicked()
{
  const int mode = m_modeCombo->currentIndex();
  const bool group = m_groupResult->isChecked();

  switch (mode)
  {
  case 0: // Linear
    m_tool.applyLinearArray(
      m_linearCount->value(),
      vm::vec3d{m_linearOffsetX->value(), m_linearOffsetY->value(), m_linearOffsetZ->value()},
      group);
    break;

  case 1: // Grid
    m_tool.applyGridArray(
      m_gridCountX->value(),
      m_gridCountY->value(),
      m_gridCountZ->value(),
      m_gridSpacingX->value(),
      m_gridSpacingY->value(),
      m_gridSpacingZ->value(),
      group);
    break;

  case 2: // Radial
    m_tool.applyRadialArray(
      m_radialCount->value(),
      vm::vec3d{m_radialCenterX->value(), m_radialCenterY->value(), m_radialCenterZ->value()},
      m_radialAxis->currentIndex(),
      m_radialAngle->value(),
      group);
    break;
  }
}

void ArrayToolPage::useSelectionCenterClicked()
{
  const auto& map = m_document.map();
  if (map.selection().hasNodes())
  {
    const auto bounds = map.selection().selectionBounds();
    const auto center = bounds.center();
    m_radialCenterX->setValue(center.x());
    m_radialCenterY->setValue(center.y());
    m_radialCenterZ->setValue(center.z());
  }
}

} // namespace tb::ui
