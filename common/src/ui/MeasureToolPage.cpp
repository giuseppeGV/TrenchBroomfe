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

#include "MeasureToolPage.h"
#include "MeasureTool.h"

#include "mdl/Map.h"
#include "mdl/Selection.h"
#include "ui/MapDocument.h"
#include "ui/SpinControl.h"
#include "ui/ViewConstants.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace tb::ui
{

MeasureToolPage::MeasureToolPage(MapDocument& document, MeasureTool& tool, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
  , m_tool{tool}
{
  createGui();
  updateMeasurements();
}

void MeasureToolPage::createGui()
{
  auto* mainLayout = new QVBoxLayout{};
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(LayoutConstants::MediumVMargin);

  // Selection Dimensions section
  {
    auto* dimGroup = new QGroupBox{tr("Selection Dimensions")};
    auto* dimLayout = new QGridLayout{};

    dimLayout->addWidget(new QLabel{tr("Width (X):")}, 0, 0);
    m_selectionDimX = new QLabel{tr("--")};
    m_selectionDimX->setTextInteractionFlags(Qt::TextSelectableByMouse);
    dimLayout->addWidget(m_selectionDimX, 0, 1);

    dimLayout->addWidget(new QLabel{tr("Depth (Y):")}, 1, 0);
    m_selectionDimY = new QLabel{tr("--")};
    m_selectionDimY->setTextInteractionFlags(Qt::TextSelectableByMouse);
    dimLayout->addWidget(m_selectionDimY, 1, 1);

    dimLayout->addWidget(new QLabel{tr("Height (Z):")}, 2, 0);
    m_selectionDimZ = new QLabel{tr("--")};
    m_selectionDimZ->setTextInteractionFlags(Qt::TextSelectableByMouse);
    dimLayout->addWidget(m_selectionDimZ, 2, 1);

    dimLayout->addWidget(new QLabel{tr("Diagonal:")}, 3, 0);
    m_selectionDiagonal = new QLabel{tr("--")};
    m_selectionDiagonal->setTextInteractionFlags(Qt::TextSelectableByMouse);
    dimLayout->addWidget(m_selectionDiagonal, 3, 1);

    dimLayout->setColumnStretch(1, 1);
    dimGroup->setLayout(dimLayout);
    mainLayout->addWidget(dimGroup);
  }

  // Object Counts section
  {
    auto* countGroup = new QGroupBox{tr("Selection Contents")};
    auto* countLayout = new QGridLayout{};

    countLayout->addWidget(new QLabel{tr("Brushes:")}, 0, 0);
    m_brushCount = new QLabel{tr("0")};
    countLayout->addWidget(m_brushCount, 0, 1);

    countLayout->addWidget(new QLabel{tr("Entities:")}, 0, 2);
    m_entityCount = new QLabel{tr("0")};
    countLayout->addWidget(m_entityCount, 0, 3);

    countLayout->addWidget(new QLabel{tr("Faces:")}, 1, 0);
    m_faceCount = new QLabel{tr("0")};
    countLayout->addWidget(m_faceCount, 1, 1);

    countLayout->addWidget(new QLabel{tr("Vertices:")}, 1, 2);
    m_vertexCount = new QLabel{tr("0")};
    countLayout->addWidget(m_vertexCount, 1, 3);

    countGroup->setLayout(countLayout);
    mainLayout->addWidget(countGroup);
  }

  // Calculated Properties section
  {
    auto* calcGroup = new QGroupBox{tr("Calculated Properties")};
    auto* calcLayout = new QGridLayout{};

    calcLayout->addWidget(new QLabel{tr("Total Face Area:")}, 0, 0);
    m_totalArea = new QLabel{tr("--")};
    m_totalArea->setTextInteractionFlags(Qt::TextSelectableByMouse);
    calcLayout->addWidget(m_totalArea, 0, 1);

    calcLayout->addWidget(new QLabel{tr("Approx. Volume:")}, 1, 0);
    m_totalVolume = new QLabel{tr("--")};
    m_totalVolume->setTextInteractionFlags(Qt::TextSelectableByMouse);
    calcLayout->addWidget(m_totalVolume, 1, 1);

    calcLayout->setColumnStretch(1, 1);
    calcGroup->setLayout(calcLayout);
    mainLayout->addWidget(calcGroup);
  }

  // Point-to-Point Measurement section
  {
    auto* pointGroup = new QGroupBox{tr("Point-to-Point Measurement")};
    auto* pointLayout = new QGridLayout{};

    auto createDoubleSpin = [this]() {
      auto* spin = new SpinControl{this};
      spin->setRange(-100000.0, 100000.0);
      spin->setValue(0.0);
      return spin;
    };

    pointLayout->addWidget(new QLabel{tr("Start X/Y/Z:")}, 0, 0);
    m_startX = createDoubleSpin();
    m_startY = createDoubleSpin();
    m_startZ = createDoubleSpin();
    pointLayout->addWidget(m_startX, 0, 1);
    pointLayout->addWidget(m_startY, 0, 2);
    pointLayout->addWidget(m_startZ, 0, 3);

    m_useSelectionStartButton = new QPushButton{tr("Use Selection Min")};
    connect(m_useSelectionStartButton, &QPushButton::clicked, this, &MeasureToolPage::useSelectionStartClicked);
    pointLayout->addWidget(m_useSelectionStartButton, 1, 1, 1, 3);

    pointLayout->addWidget(new QLabel{tr("End X/Y/Z:")}, 2, 0);
    m_endX = createDoubleSpin();
    m_endY = createDoubleSpin();
    m_endZ = createDoubleSpin();
    pointLayout->addWidget(m_endX, 2, 1);
    pointLayout->addWidget(m_endY, 2, 2);
    pointLayout->addWidget(m_endZ, 2, 3);

    m_useSelectionEndButton = new QPushButton{tr("Use Selection Max")};
    connect(m_useSelectionEndButton, &QPushButton::clicked, this, &MeasureToolPage::useSelectionEndClicked);
    pointLayout->addWidget(m_useSelectionEndButton, 3, 1, 1, 3);

    pointLayout->addWidget(new QLabel{tr("Distance:")}, 4, 0);
    m_pointDistance = new QLabel{tr("0.0")};
    m_pointDistance->setTextInteractionFlags(Qt::TextSelectableByMouse);
    pointLayout->addWidget(m_pointDistance, 4, 1, 1, 3);

    pointLayout->addWidget(new QLabel{tr("X/Y/Z:")}, 5, 0);
    m_pointDistX = new QLabel{tr("0.0")};
    m_pointDistY = new QLabel{tr("0.0")};
    m_pointDistZ = new QLabel{tr("0.0")};
    m_pointDistX->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_pointDistY->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_pointDistZ->setTextInteractionFlags(Qt::TextSelectableByMouse);
    pointLayout->addWidget(m_pointDistX, 5, 1);
    pointLayout->addWidget(m_pointDistY, 5, 2);
    pointLayout->addWidget(m_pointDistZ, 5, 3);

    // Connect spin changes
    connect(m_startX, &SpinControl::valueChanged, this, &MeasureToolPage::pointChanged);
    connect(m_startY, &SpinControl::valueChanged, this, &MeasureToolPage::pointChanged);
    connect(m_startZ, &SpinControl::valueChanged, this, &MeasureToolPage::pointChanged);
    connect(m_endX, &SpinControl::valueChanged, this, &MeasureToolPage::pointChanged);
    connect(m_endY, &SpinControl::valueChanged, this, &MeasureToolPage::pointChanged);
    connect(m_endZ, &SpinControl::valueChanged, this, &MeasureToolPage::pointChanged);

    pointGroup->setLayout(pointLayout);
    mainLayout->addWidget(pointGroup);
  }

  // Refresh button
  m_refreshButton = new QPushButton{tr("Refresh Measurements")};
  connect(m_refreshButton, &QPushButton::clicked, this, &MeasureToolPage::refreshClicked);
  mainLayout->addWidget(m_refreshButton);

  mainLayout->addStretch(1);

  setLayout(mainLayout);
}

void MeasureToolPage::updateMeasurements()
{
  const auto result = m_tool.measureSelection();

  // Update dimension labels
  m_selectionDimX->setText(QString::number(result.distanceX, 'f', 2));
  m_selectionDimY->setText(QString::number(result.distanceY, 'f', 2));
  m_selectionDimZ->setText(QString::number(result.distanceZ, 'f', 2));
  m_selectionDiagonal->setText(QString::number(result.totalDistance, 'f', 2));

  // Update counts
  m_brushCount->setText(QString::number(result.brushCount));
  m_entityCount->setText(QString::number(result.entityCount));
  m_faceCount->setText(QString::number(result.faceCount));
  m_vertexCount->setText(QString::number(result.vertexCount));

  // Update calculated values
  m_totalArea->setText(QString::number(result.area, 'f', 2) + tr(" sq units"));
  m_totalVolume->setText(QString::number(result.volume, 'f', 2) + tr(" cu units"));

  // Update point-to-point measurement
  pointChanged();
}

void MeasureToolPage::refreshClicked()
{
  updateMeasurements();
}

void MeasureToolPage::useSelectionStartClicked()
{
  const auto& map = m_document.map();
  if (map.selection().hasNodes())
  {
    const auto bounds = *map.selectionBounds();
    m_startX->setValue(bounds.min.x());
    m_startY->setValue(bounds.min.y());
    m_startZ->setValue(bounds.min.z());
    pointChanged();
  }
}

void MeasureToolPage::useSelectionEndClicked()
{
  const auto& map = m_document.map();
  if (map.selection().hasNodes())
  {
    const auto bounds = *map.selectionBounds();
    m_endX->setValue(bounds.max.x());
    m_endY->setValue(bounds.max.y());
    m_endZ->setValue(bounds.max.z());
    pointChanged();
  }
}

void MeasureToolPage::pointChanged()
{
  const vm::vec3d start{m_startX->value(), m_startY->value(), m_startZ->value()};
  const vm::vec3d end{m_endX->value(), m_endY->value(), m_endZ->value()};
  
  m_tool.setStartPoint(start);
  m_tool.setEndPoint(end);

  const double distance = m_tool.calculateDistance();
  const auto components = m_tool.calculateComponentDistances();

  m_pointDistance->setText(QString::number(distance, 'f', 2));
  m_pointDistX->setText(QString::number(components.x(), 'f', 2));
  m_pointDistY->setText(QString::number(components.y(), 'f', 2));
  m_pointDistZ->setText(QString::number(components.z(), 'f', 2));
}

} // namespace tb::ui
