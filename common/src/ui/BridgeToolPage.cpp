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

#include "BridgeToolPage.h"
#include "BridgeTool.h"

#include "mdl/Map.h"
#include "mdl/Selection.h"
#include "ui/MapDocument.h"
#include "ui/SpinControl.h"
#include "ui/ViewConstants.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace tb::ui
{

BridgeToolPage::BridgeToolPage(MapDocument& document, BridgeTool& tool, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
  , m_tool{tool}
{
  createGui();
  updateGui();
}

void BridgeToolPage::createGui()
{
  auto* mainLayout = new QVBoxLayout{};
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(LayoutConstants::MediumVMargin);

  // Instructions
  m_statusLabel = new QLabel{tr("Select two brush faces to create a bridge between them.")};
  m_statusLabel->setWordWrap(true);
  mainLayout->addWidget(m_statusLabel);

  // Settings group
  auto* settingsGroup = new QGroupBox{tr("Bridge Settings")};
  auto* settingsLayout = new QGridLayout{};

  int row = 0;

  // Segments
  settingsLayout->addWidget(new QLabel{tr("Segments:")}, row, 0);
  m_segmentsSpinner = new QSpinBox{};
  m_segmentsSpinner->setRange(1, 32);
  m_segmentsSpinner->setValue(m_tool.segments());
  connect(m_segmentsSpinner, QOverload<int>::of(&QSpinBox::valueChanged),
          this, &BridgeToolPage::segmentsChanged);
  settingsLayout->addWidget(m_segmentsSpinner, row, 1);
  row++;

  // Curved bridge
  m_curvedCheck = new QCheckBox{tr("Curved bridge")};
  m_curvedCheck->setChecked(m_tool.isCurved());
  connect(m_curvedCheck, &QCheckBox::toggled, this, &BridgeToolPage::curvedChanged);
  settingsLayout->addWidget(m_curvedCheck, row, 0, 1, 2);
  row++;

  // Curvature amount
  settingsLayout->addWidget(new QLabel{tr("Curvature:")}, row, 0);
  m_curvatureSpin = new SpinControl{this};
  m_curvatureSpin->setRange(-1.0, 1.0);
  m_curvatureSpin->setValue(m_tool.curvature());
  m_curvatureSpin->setEnabled(m_tool.isCurved());
  connect(m_curvatureSpin, &SpinControl::valueChanged, this, &BridgeToolPage::curvatureChanged);
  settingsLayout->addWidget(m_curvatureSpin, row, 1);
  row++;

  // Taper
  m_taperCheck = new QCheckBox{tr("Taper bridge")};
  m_taperCheck->setChecked(m_tool.isTapered());
  connect(m_taperCheck, &QCheckBox::toggled, this, &BridgeToolPage::taperChanged);
  settingsLayout->addWidget(m_taperCheck, row, 0, 1, 2);
  row++;

  // Taper amount
  settingsLayout->addWidget(new QLabel{tr("Taper Amount:")}, row, 0);
  m_taperAmountSpin = new SpinControl{this};
  m_taperAmountSpin->setRange(-1.0, 1.0);
  m_taperAmountSpin->setValue(m_tool.taperAmount());
  m_taperAmountSpin->setEnabled(m_tool.isTapered());
  connect(m_taperAmountSpin, &SpinControl::valueChanged, this, &BridgeToolPage::taperAmountChanged);
  settingsLayout->addWidget(m_taperAmountSpin, row, 1);
  row++;

  settingsGroup->setLayout(settingsLayout);
  mainLayout->addWidget(settingsGroup);

  // Create button
  m_createButton = new QPushButton{tr("Create Bridge")};
  m_createButton->setDefault(true);
  connect(m_createButton, &QPushButton::clicked, this, &BridgeToolPage::createBridgeClicked);
  mainLayout->addWidget(m_createButton);

  mainLayout->addStretch(1);

  setLayout(mainLayout);
}

void BridgeToolPage::updateGui()
{
  const bool canCreate = m_tool.canCreateBridge();
  m_createButton->setEnabled(canCreate);
  
  if (canCreate)
  {
    m_statusLabel->setText(tr("Ready to create bridge between the two selected faces."));
    m_statusLabel->setStyleSheet("color: #a6e3a1;"); // Green
  }
  else
  {
    const auto& map = m_document.map();
    const auto faceCount = map.selection().brushFaces.size();
    
    if (faceCount == 0)
    {
      m_statusLabel->setText(tr("Select two brush faces to create a bridge."));
    }
    else if (faceCount == 1)
    {
      m_statusLabel->setText(tr("Select one more face. (1 of 2 selected)"));
    }
    else
    {
      m_statusLabel->setText(tr("Too many faces selected. Select exactly two faces."));
    }
    m_statusLabel->setStyleSheet("color: #f9e2af;"); // Yellow
  }
}

void BridgeToolPage::segmentsChanged(int value)
{
  m_tool.setSegments(value);
}

void BridgeToolPage::curvedChanged(bool checked)
{
  m_tool.setCurved(checked);
  m_curvatureSpin->setEnabled(checked);
}

void BridgeToolPage::curvatureChanged(double value)
{
  m_tool.setCurvature(value);
}

void BridgeToolPage::taperChanged(bool checked)
{
  m_tool.setTaper(checked);
  m_taperAmountSpin->setEnabled(checked);
}

void BridgeToolPage::taperAmountChanged(double value)
{
  m_tool.setTaperAmount(value);
}

void BridgeToolPage::createBridgeClicked()
{
  if (m_tool.createBridge())
  {
    updateGui();
  }
}

} // namespace tb::ui
