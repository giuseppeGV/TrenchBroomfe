#pragma once

#include <QDialog>
#include <QString>

class QComboBox;
class QDialogButtonBox;

namespace tb::ui
{
class MapDocument;

class SearchByTextureDialog : public QDialog
{
  Q_OBJECT

private:
  MapDocument& m_document;
  QComboBox* m_textureInput = nullptr;
  QDialogButtonBox* m_buttons = nullptr;

public:
  SearchByTextureDialog(MapDocument& document, QWidget* parent = nullptr);

  QString textureName() const;
  
private:
  void createGui();
  void populateTextures();
};

} // namespace tb::ui
