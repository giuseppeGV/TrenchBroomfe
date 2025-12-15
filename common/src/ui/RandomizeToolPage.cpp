#include "RandomizeToolPage.h"
#include "RandomizeTool.h"

#include "ui/MapDocument.h"
#include "ui/SpinControl.h"
#include "ui/ViewConstants.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

namespace tb::ui
{

RandomizeToolPage::RandomizeToolPage(MapDocument& document, RandomizeTool& tool, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
  , m_tool{tool}
{
  createGui();
  updateGui();
}

void RandomizeToolPage::createGui()
{
  auto* layout = new QGridLayout{};
  layout->setContentsMargins(0, 0, 0, 0);

  int row = 0;

  auto createDoubleSpin = [&](double val) {
    auto* spin = new SpinControl{this};
    spin->setRange(-10000.0, 10000.0);
    spin->setValue(val);
    return spin;
  };

  // Translation
  layout->addWidget(new QLabel{tr("Translation")}, row++, 0, 1, 3);
  
  layout->addWidget(new QLabel{tr("Min X/Y/Z")}, row, 0);
  m_minTranslateX = createDoubleSpin(0.0);
  m_minTranslateY = createDoubleSpin(0.0);
  m_minTranslateZ = createDoubleSpin(0.0);
  layout->addWidget(m_minTranslateX, row, 1);
  layout->addWidget(m_minTranslateY, row, 2);
  layout->addWidget(m_minTranslateZ, row, 3);
  row++;

  layout->addWidget(new QLabel{tr("Max X/Y/Z")}, row, 0);
  m_maxTranslateX = createDoubleSpin(0.0);
  m_maxTranslateY = createDoubleSpin(0.0);
  m_maxTranslateZ = createDoubleSpin(0.0);
  layout->addWidget(m_maxTranslateX, row, 1);
  layout->addWidget(m_maxTranslateY, row, 2);
  layout->addWidget(m_maxTranslateZ, row, 3);
  row++;

  // Rotation
  layout->addWidget(new QLabel{tr("Rotation (Deg)")}, row++, 0, 1, 3);
  
  layout->addWidget(new QLabel{tr("Min X/Y/Z")}, row, 0);
  m_minRotateX = createDoubleSpin(0.0);
  m_minRotateY = createDoubleSpin(0.0);
  m_minRotateZ = createDoubleSpin(0.0);
  layout->addWidget(m_minRotateX, row, 1);
  layout->addWidget(m_minRotateY, row, 2);
  layout->addWidget(m_minRotateZ, row, 3);
  row++;

  layout->addWidget(new QLabel{tr("Max X/Y/Z")}, row, 0);
  m_maxRotateX = createDoubleSpin(0.0);
  m_maxRotateY = createDoubleSpin(0.0);
  m_maxRotateZ = createDoubleSpin(0.0);
  layout->addWidget(m_maxRotateX, row, 1);
  layout->addWidget(m_maxRotateY, row, 2);
  layout->addWidget(m_maxRotateZ, row, 3);
  row++;

  // Scale
  layout->addWidget(new QLabel{tr("Scale")}, row++, 0, 1, 3);
  
  auto createScaleSpin = [&](double val) {
    auto* spin = new SpinControl{this};
    spin->setRange(0.1, 100.0);
    spin->setValue(val);
    return spin;
  };

  layout->addWidget(new QLabel{tr("Min X/Y/Z")}, row, 0);
  m_minScaleX = createScaleSpin(1.0);
  m_minScaleY = createScaleSpin(1.0);
  m_minScaleZ = createScaleSpin(1.0);
  layout->addWidget(m_minScaleX, row, 1);
  layout->addWidget(m_minScaleY, row, 2);
  layout->addWidget(m_minScaleZ, row, 3);
  row++;

  layout->addWidget(new QLabel{tr("Max X/Y/Z")}, row, 0);
  m_maxScaleX = createScaleSpin(1.0);
  m_maxScaleY = createScaleSpin(1.0);
  m_maxScaleZ = createScaleSpin(1.0);
  layout->addWidget(m_maxScaleX, row, 1);
  layout->addWidget(m_maxScaleY, row, 2);
  layout->addWidget(m_maxScaleZ, row, 3);
  row++;

  m_applyButton = new QPushButton{tr("Apply Randomization")};
  layout->addWidget(m_applyButton, row++, 0, 1, 4);

  connect(m_applyButton, &QPushButton::clicked, this, &RandomizeToolPage::applyClicked);

  layout->setColumnStretch(1, 1);
  layout->setColumnStretch(2, 1);
  layout->setColumnStretch(3, 1);
  
  setLayout(layout);
}

void RandomizeToolPage::updateGui()
{
}

void RandomizeToolPage::applyClicked()
{
  m_tool.applyRandomization(
    vm::vec3d{m_minTranslateX->value(), m_minTranslateY->value(), m_minTranslateZ->value()},
    vm::vec3d{m_maxTranslateX->value(), m_maxTranslateY->value(), m_maxTranslateZ->value()},
    vm::vec3d{m_minRotateX->value(), m_minRotateY->value(), m_minRotateZ->value()},
    vm::vec3d{m_maxRotateX->value(), m_maxRotateY->value(), m_maxRotateZ->value()},
    vm::vec3d{m_minScaleX->value(), m_minScaleY->value(), m_minScaleZ->value()},
    vm::vec3d{m_maxScaleX->value(), m_maxScaleY->value(), m_maxScaleZ->value()}
  );
}

} // namespace tb::ui
