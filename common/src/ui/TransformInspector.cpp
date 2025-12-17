#include "TransformInspector.h"

#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/SelectionChange.h"
#include "ui/MapDocument.h"
#include "ui/QtUtils.h"

#include "vm/mat.h"
#include "vm/vec.h"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <QCheckBox>
#include <QComboBox>

namespace tb::ui
{

TransformInspector::TransformInspector(MapDocument& document, QWidget* parent)
  : TabBookPage{parent}
  , m_document{document}
{
  createGui();
  connectSignals();
  updateUi();
}

TransformInspector::~TransformInspector() = default;

void TransformInspector::createGui()
{
  auto* layout = new QVBoxLayout{this};
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(8);

  // Position Group
  {
    auto* group = new QVBoxLayout{};
    group->addWidget(new QLabel{"<b>Position</b>"});

    auto* row = new QHBoxLayout{};
    // row->addWidget(new QLabel{"Position:"}); // Removing redundant label
    
    m_xInput = new QDoubleSpinBox{};
    m_xInput->setRange(-100000.0, 100000.0);
    m_xInput->setDecimals(2);
    m_xInput->setPrefix("X: ");
    
    m_yInput = new QDoubleSpinBox{};
    m_yInput->setRange(-100000.0, 100000.0);
    m_yInput->setDecimals(2);
    m_yInput->setPrefix("Y: ");

    m_zInput = new QDoubleSpinBox{};
    m_zInput->setRange(-100000.0, 100000.0);
    m_zInput->setDecimals(2);
    m_zInput->setPrefix("Z: ");

    row->addWidget(m_xInput);
    row->addWidget(m_yInput);
    row->addWidget(m_zInput);
    group->addLayout(row);

    m_applyButton = new QPushButton{"Apply Move"};
    group->addWidget(m_applyButton);
    
    layout->addLayout(group);
  }

  // Symmetry Group
  {
      layout->addSpacing(10);
      auto* group = new QVBoxLayout{};
      group->addWidget(new QLabel{"<b>Live Symmetry</b>"});
      
      m_symmetryEnabled = new QCheckBox{"Enable Symmetry"};
      group->addWidget(m_symmetryEnabled);
      
      auto* row = new QHBoxLayout{};
      row->addWidget(new QLabel{"Axis:"});
      m_symmetryAxis = new QComboBox{};
      m_symmetryAxis->addItem("X-Axis");
      m_symmetryAxis->addItem("Y-Axis");
      m_symmetryAxis->addItem("Z-Axis");
      row->addWidget(m_symmetryAxis);
      
      group->addLayout(row);
      layout->addLayout(group);
  }

  // Repair Group
  {
      layout->addSpacing(10);
      auto* group = new QVBoxLayout{};
      group->addWidget(new QLabel{"<b>Tools</b>"});
      
      m_repairButton = new QPushButton{"Repair Shape"};
      m_repairButton->setToolTip("Repairs selected brushes by converting them to their convex hull.");
      group->addWidget(m_repairButton);
      
      layout->addLayout(group);
  }
  
  layout->addStretch(1);
}

void TransformInspector::connectSignals()
{
  m_selectionConnection = m_document.selectionDidChangeNotifier.connect(
    [this](const mdl::SelectionChange&) { updateUi(); });
    
  m_symmetryConnection = m_document.symmetryDidChangeNotifier.connect(
    [this]() { updateUi(); });

  connect(m_applyButton, &QPushButton::clicked, this, &TransformInspector::applyTransform);
  connect(m_repairButton, &QPushButton::clicked, this, &TransformInspector::repairConvexity);
  
  connect(m_symmetryEnabled, &QCheckBox::toggled, this, [this](bool checked){
      if (m_updatingUi) return;
      m_document.symmetryManager().setEnabled(checked);
      m_document.symmetryDidChangeNotifier();
  });
  
  connect(m_symmetryAxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
      if (m_updatingUi) return;
      // 0=X, 1=Y, 2=Z match enum
      m_document.symmetryManager().setAxis(static_cast<mdl::SymmetryAxis>(index));
      m_document.symmetryDidChangeNotifier();
  });
}

void TransformInspector::updateUi()
{
  m_updatingUi = true;
  
  bool hasSelection = false;
  // Transform UI
  if (const auto bounds = m_document.map().selectionBounds())
  {
      hasSelection = true;
      const auto center = bounds->center();
      m_xInput->setValue(center.x());
      m_yInput->setValue(center.y());
      m_zInput->setValue(center.z());
      
      m_xInput->setEnabled(true);
      m_yInput->setEnabled(true);
      m_zInput->setEnabled(true);
      m_applyButton->setEnabled(true);
  }
  else
  {
      m_xInput->clear();
      m_yInput->clear();
      m_zInput->clear();
      
      m_xInput->setEnabled(false);
      m_yInput->setEnabled(false);
      m_zInput->setEnabled(false);
      m_applyButton->setEnabled(false);
  }

  // Symmetry UI
  const auto& sm = m_document.symmetryManager();
  m_symmetryEnabled->setChecked(sm.isEnabled());
  m_symmetryAxis->setCurrentIndex(static_cast<int>(sm.axis()));
  m_symmetryAxis->setEnabled(sm.isEnabled());

  // Repair UI
  m_repairButton->setEnabled(hasSelection);

  m_updatingUi = false;
}

void TransformInspector::applyTransform()
{
  if (m_updatingUi) return;

  if (const auto bounds = m_document.map().selectionBounds())
  {
      const auto currentCenter = bounds->center();
      const auto newCenter = vm::vec3d{
          m_xInput->value(),
          m_yInput->value(),
          m_zInput->value()
      };
      
      if (vm::is_equal(currentCenter, newCenter, 0.001))
      {
          return;
      }

      const auto delta = newCenter - currentCenter;
      
      m_document.map().startTransaction("Numeric Transform", mdl::TransactionScope::LongRunning);
      if (mdl::translateSelection(m_document.map(), delta))
      {
          m_document.map().commitTransaction();
      }
      else
      {
          m_document.map().cancelTransaction();
      }
  }
}

void TransformInspector::repairConvexity()
{
    if (mdl::repairConvexity(m_document.map()))
    {
        // Success
    }
}

} // namespace tb::ui


