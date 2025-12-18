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

#include "AlignmentToolPage.h"
#include "AlignmentTool.h"

#include "ui/MapDocument.h"
#include "ui/SpinControl.h"
#include "ui/ViewConstants.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace tb::ui
{

AlignmentToolPage::AlignmentToolPage(MapDocument& document, AlignmentTool& tool, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
  , m_tool{tool}
{
  createGui();
  updateGui();
}

void AlignmentToolPage::createGui()
{
  auto* mainLayout = new QVBoxLayout{};
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(LayoutConstants::MediumVMargin);

  // Align section
  {
    auto* alignGroup = new QGroupBox{tr("Align")};
    auto* alignLayout = new QGridLayout{};
    
    alignLayout->addWidget(new QLabel{tr("Axis:")}, 0, 0);
    m_alignAxis = new QComboBox{};
    m_alignAxis->addItems({tr("X"), tr("Y"), tr("Z")});
    alignLayout->addWidget(m_alignAxis, 0, 1);

    alignLayout->addWidget(new QLabel{tr("Mode:")}, 1, 0);
    m_alignMode = new QComboBox{};
    m_alignMode->addItems({tr("Min (Left/Bottom/Back)"), tr("Center"), tr("Max (Right/Top/Front)")});
    m_alignMode->setCurrentIndex(1);
    alignLayout->addWidget(m_alignMode, 1, 1);

    m_alignToFirst = new QCheckBox{tr("Align to first selected")};
    alignLayout->addWidget(m_alignToFirst, 2, 0, 1, 2);

    m_alignButton = new QPushButton{tr("Align Objects")};
    connect(m_alignButton, &QPushButton::clicked, this, &AlignmentToolPage::alignClicked);
    alignLayout->addWidget(m_alignButton, 3, 0, 1, 2);

    alignGroup->setLayout(alignLayout);
    mainLayout->addWidget(alignGroup);
  }

  // Distribute section
  {
    auto* distributeGroup = new QGroupBox{tr("Distribute")};
    auto* distributeLayout = new QGridLayout{};
    
    distributeLayout->addWidget(new QLabel{tr("Axis:")}, 0, 0);
    m_distributeAxis = new QComboBox{};
    m_distributeAxis->addItems({tr("X"), tr("Y"), tr("Z")});
    distributeLayout->addWidget(m_distributeAxis, 0, 1);

    m_useSpacing = new QCheckBox{tr("Use fixed spacing:")};
    m_spacing = new SpinControl{this};
    m_spacing->setRange(0.0, 10000.0);
    m_spacing->setValue(8.0);
    m_spacing->setEnabled(false);
    connect(m_useSpacing, &QCheckBox::toggled, m_spacing, &SpinControl::setEnabled);
    distributeLayout->addWidget(m_useSpacing, 1, 0);
    distributeLayout->addWidget(m_spacing, 1, 1);

    m_distributeButton = new QPushButton{tr("Distribute Objects")};
    connect(m_distributeButton, &QPushButton::clicked, this, &AlignmentToolPage::distributeClicked);
    distributeLayout->addWidget(m_distributeButton, 2, 0, 1, 2);

    distributeGroup->setLayout(distributeLayout);
    mainLayout->addWidget(distributeGroup);
  }

  // Align to Grid section
  {
    auto* gridGroup = new QGroupBox{tr("Align to Grid")};
    auto* gridLayout = new QGridLayout{};
    
    gridLayout->addWidget(new QLabel{tr("Axis:")}, 0, 0);
    m_gridAxis = new QComboBox{};
    m_gridAxis->addItems({tr("All Axes"), tr("X"), tr("Y"), tr("Z")});
    gridLayout->addWidget(m_gridAxis, 0, 1);

    gridLayout->addWidget(new QLabel{tr("Mode:")}, 1, 0);
    m_gridMode = new QComboBox{};
    m_gridMode->addItems({tr("Min"), tr("Center"), tr("Max")});
    gridLayout->addWidget(m_gridMode, 1, 1);

    m_gridButton = new QPushButton{tr("Align to Grid")};
    connect(m_gridButton, &QPushButton::clicked, this, &AlignmentToolPage::alignToGridClicked);
    gridLayout->addWidget(m_gridButton, 2, 0, 1, 2);

    gridGroup->setLayout(gridLayout);
    mainLayout->addWidget(gridGroup);
  }

  // Stack section
  {
    auto* stackGroup = new QGroupBox{tr("Stack")};
    auto* stackLayout = new QGridLayout{};
    
    stackLayout->addWidget(new QLabel{tr("Axis:")}, 0, 0);
    m_stackAxis = new QComboBox{};
    m_stackAxis->addItems({tr("X"), tr("Y"), tr("Z")});
    m_stackAxis->setCurrentIndex(2); // Z axis default
    stackLayout->addWidget(m_stackAxis, 0, 1);

    stackLayout->addWidget(new QLabel{tr("Gap:")}, 1, 0);
    m_stackGap = new SpinControl{this};
    m_stackGap->setRange(0.0, 10000.0);
    m_stackGap->setValue(0.0);
    stackLayout->addWidget(m_stackGap, 1, 1);

    m_stackButton = new QPushButton{tr("Stack Objects")};
    connect(m_stackButton, &QPushButton::clicked, this, &AlignmentToolPage::stackClicked);
    stackLayout->addWidget(m_stackButton, 2, 0, 1, 2);

    stackGroup->setLayout(stackLayout);
    mainLayout->addWidget(stackGroup);
  }

  mainLayout->addStretch(1);

  setLayout(mainLayout);
}

void AlignmentToolPage::updateGui()
{
}

void AlignmentToolPage::alignClicked()
{
  const int axis = m_alignAxis->currentIndex();
  const AlignMode mode = static_cast<AlignMode>(m_alignMode->currentIndex());
  const bool alignToFirst = m_alignToFirst->isChecked();
  
  m_tool.alignObjects(axis, mode, alignToFirst);
}

void AlignmentToolPage::distributeClicked()
{
  const int axis = m_distributeAxis->currentIndex();
  const bool useSpacing = m_useSpacing->isChecked();
  const double spacing = m_spacing->value();
  
  m_tool.distributeObjects(axis, useSpacing, spacing);
}

void AlignmentToolPage::alignToGridClicked()
{
  const int axisIndex = m_gridAxis->currentIndex();
  const int axis = (axisIndex == 0) ? -1 : (axisIndex - 1);
  const AlignMode mode = static_cast<AlignMode>(m_gridMode->currentIndex());
  
  m_tool.alignToGrid(axis, mode);
}

void AlignmentToolPage::stackClicked()
{
  const int axis = m_stackAxis->currentIndex();
  const double gap = m_stackGap->value();
  
  m_tool.stackObjects(axis, gap);
}

} // namespace tb::ui
