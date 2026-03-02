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

#include "ui/ArrayDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "ui/MapDocument.h"
#include "ui/QtUtils.h"

#include "vm/vec.h"

namespace tb::ui
{

ArrayDialog::ArrayDialog(MapDocument& document, QWidget* parent)
  : QDialog{parent}
  , m_document{document}
{
  createGui();
}

void ArrayDialog::createGui()
{
  setWindowIconTB(this);
  setWindowTitle(tr("Array / Clone"));
  setMinimumWidth(350);

  // Mode selector
  m_modeCombo = new QComboBox{};
  m_modeCombo->addItem(tr("Linear"));
  m_modeCombo->addItem(tr("Radial"));

  // Count
  m_countSpin = new QSpinBox{};
  m_countSpin->setRange(1, 256);
  m_countSpin->setValue(4);

  // ---------- Linear page ----------
  auto* linearPage = new QWidget{};
  {
    m_offsetX = new QDoubleSpinBox{};
    m_offsetX->setRange(-65536.0, 65536.0);
    m_offsetX->setValue(64.0);
    m_offsetX->setDecimals(1);

    m_offsetY = new QDoubleSpinBox{};
    m_offsetY->setRange(-65536.0, 65536.0);
    m_offsetY->setValue(0.0);
    m_offsetY->setDecimals(1);

    m_offsetZ = new QDoubleSpinBox{};
    m_offsetZ->setRange(-65536.0, 65536.0);
    m_offsetZ->setValue(0.0);
    m_offsetZ->setDecimals(1);

    auto* form = new QFormLayout{};
    form->addRow(tr("Offset X:"), m_offsetX);
    form->addRow(tr("Offset Y:"), m_offsetY);
    form->addRow(tr("Offset Z:"), m_offsetZ);
    linearPage->setLayout(form);
  }

  // ---------- Radial page ----------
  auto* radialPage = new QWidget{};
  {
    m_centerX = new QDoubleSpinBox{};
    m_centerX->setRange(-65536.0, 65536.0);
    m_centerX->setValue(0.0);
    m_centerX->setDecimals(1);

    m_centerY = new QDoubleSpinBox{};
    m_centerY->setRange(-65536.0, 65536.0);
    m_centerY->setValue(0.0);
    m_centerY->setDecimals(1);

    m_centerZ = new QDoubleSpinBox{};
    m_centerZ->setRange(-65536.0, 65536.0);
    m_centerZ->setValue(0.0);
    m_centerZ->setDecimals(1);

    m_axisCombo = new QComboBox{};
    m_axisCombo->addItem(tr("X"));
    m_axisCombo->addItem(tr("Y"));
    m_axisCombo->addItem(tr("Z"));
    m_axisCombo->setCurrentIndex(2); // Z by default

    m_totalAngle = new QDoubleSpinBox{};
    m_totalAngle->setRange(0.0, 360.0);
    m_totalAngle->setValue(360.0);
    m_totalAngle->setDecimals(1);
    m_totalAngle->setSuffix(tr(" °"));

    auto* form = new QFormLayout{};
    form->addRow(tr("Center X:"), m_centerX);
    form->addRow(tr("Center Y:"), m_centerY);
    form->addRow(tr("Center Z:"), m_centerZ);
    form->addRow(tr("Axis:"), m_axisCombo);
    form->addRow(tr("Total Angle:"), m_totalAngle);
    radialPage->setLayout(form);
  }

  // Stacked widget for mode pages
  m_modeStack = new QStackedWidget{};
  m_modeStack->addWidget(linearPage);
  m_modeStack->addWidget(radialPage);

  connect(
    m_modeCombo,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    &ArrayDialog::modeChanged);

  // Buttons
  auto* buttonBox = new QDialogButtonBox{};
  auto* okButton = buttonBox->addButton(QDialogButtonBox::Ok);
  buttonBox->addButton(QDialogButtonBox::Cancel);
  okButton->setText(tr("Create Array"));

  connect(buttonBox, &QDialogButtonBox::accepted, this, &ArrayDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  // Layout
  auto* topForm = new QFormLayout{};
  topForm->addRow(tr("Mode:"), m_modeCombo);
  topForm->addRow(tr("Count:"), m_countSpin);

  auto* layout = new QVBoxLayout{};
  layout->addLayout(topForm);
  layout->addWidget(m_modeStack);
  layout->addWidget(buttonBox);
  setLayout(layout);
}

void ArrayDialog::modeChanged(const int index)
{
  m_modeStack->setCurrentIndex(index);
}

void ArrayDialog::accept()
{
  auto& map = m_document.map();
  const auto count = static_cast<size_t>(m_countSpin->value());

  if (m_modeCombo->currentIndex() == 0)
  {
    // Linear
    const auto offset =
      vm::vec3d{m_offsetX->value(), m_offsetY->value(), m_offsetZ->value()};
    mdl::arrayLinear(map, count, offset);
  }
  else
  {
    // Radial
    const auto center =
      vm::vec3d{m_centerX->value(), m_centerY->value(), m_centerZ->value()};
    const auto axisIndex = m_axisCombo->currentIndex();
    const auto axis =
      axisIndex == 0 ? vm::axis::x : (axisIndex == 1 ? vm::axis::y : vm::axis::z);
    const auto angle = m_totalAngle->value();
    mdl::arrayRadial(map, count, center, axis, angle);
  }

  QDialog::accept();
}

} // namespace tb::ui
