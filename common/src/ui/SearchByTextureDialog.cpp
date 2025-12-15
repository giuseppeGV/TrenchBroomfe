#include "SearchByTextureDialog.h"

#include "mdl/Map.h"
#include "mdl/MaterialManager.h"
#include "ui/MapDocument.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace tb::ui
{

SearchByTextureDialog::SearchByTextureDialog(MapDocument& document, QWidget* parent)
  : QDialog{parent}
  , m_document{document}
{
  setWindowTitle(tr("Search by Texture"));
  createGui();
  populateTextures();
}

void SearchByTextureDialog::createGui()
{
  auto* layout = new QVBoxLayout{};

  auto* formLayout = new QFormLayout{};
  m_textureInput = new QComboBox{};
  m_textureInput->setEditable(true);
  formLayout->addRow(tr("Texture Name:"), m_textureInput);
  layout->addLayout(formLayout);

  m_buttons = new QDialogButtonBox{
    QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this};
  connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addWidget(m_buttons);

  setLayout(layout);
}

void SearchByTextureDialog::populateTextures()
{
  // Populate with currently used textures in the map if possible
  // accessing m_document.map().materialManager()...
  // But MaterialManager might track resources, not just names usage.
  
  // For now, let's just leave it empty or prefill if we have something selected.
  // We can populate with "Recent" or "Used" textures.
}

QString SearchByTextureDialog::textureName() const
{
  return m_textureInput->currentText();
}

} // namespace tb::ui
