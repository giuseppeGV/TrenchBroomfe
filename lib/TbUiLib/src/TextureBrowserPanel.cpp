/*
 Copyright (C) 2010 Kristian Duske

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

#include "ui/TextureBrowserPanel.h"

#include <QComboBox>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

#include "PreferenceManager.h"
#include "Preferences.h"
#include "gl/Material.h"
#include "gl/MaterialCollection.h"
#include "gl/MaterialManager.h"
#include "gl/Texture.h"
#include "mdl/GameInfo.h"
#include "mdl/Map.h"
#include "mdl/Map_Assets.h"
#include "ui/MapDocument.h"
#include "ui/SearchBox.h"
#include "ui/ViewConstants.h"

#include "kd/string_compare.h"

#include <algorithm>
#include <ranges>

namespace tb::ui
{

// Supported extensions for external directory scanning.
// Qt's QImage supports these natively on most platforms.
const std::vector<std::string> TextureBrowserPanel::s_supportedExtensions = {
  ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".tif", ".tiff", ".gif", ".webp",
};

TextureBrowserPanel::TextureBrowserPanel(MapDocument& document, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
{
  createGui();
  connectObservers();
  populateDirectoryCombo();
  reloadGrid();
}

// ---------------------------------------------------------------------------
// GUI construction
// ---------------------------------------------------------------------------

void TextureBrowserPanel::createGui()
{
  // --- Directory row ---
  m_directoryCombo = new QComboBox{};
  m_directoryCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_directoryCombo->setToolTip(tr("Select a texture directory or collection"));

  m_browseButton = new QPushButton{tr("Browse...")};
  m_browseButton->setToolTip(tr("Add an external texture directory from disk"));

  m_refreshButton = new QPushButton{tr("Refresh")};
  m_refreshButton->setToolTip(tr("Reload textures from the current directory"));

  auto* dirLayout = new QHBoxLayout{};
  dirLayout->setContentsMargins(0, 0, 0, 0);
  dirLayout->setSpacing(LayoutConstants::NarrowHMargin);
  dirLayout->addWidget(m_directoryCombo, 1);
  dirLayout->addWidget(m_browseButton, 0);
  dirLayout->addWidget(m_refreshButton, 0);

  // --- Filter / icon-size row ---
  m_filterBox = createSearchBox();
  m_filterBox->setPlaceholderText(tr("Filter textures..."));

  m_iconSizeSlider = new QSlider{Qt::Horizontal};
  m_iconSizeSlider->setRange(48, 256);
  m_iconSizeSlider->setValue(m_thumbnailSize);
  m_iconSizeSlider->setFixedWidth(100);
  m_iconSizeSlider->setToolTip(tr("Thumbnail size"));

  auto* filterLayout = new QHBoxLayout{};
  filterLayout->setContentsMargins(0, 0, 0, 0);
  filterLayout->setSpacing(LayoutConstants::NarrowHMargin);
  filterLayout->addWidget(m_filterBox, 1);
  filterLayout->addWidget(new QLabel{tr("Size:")}, 0);
  filterLayout->addWidget(m_iconSizeSlider, 0);

  // --- Texture grid ---
  m_textureGrid = new QListWidget{};
  m_textureGrid->setViewMode(QListView::IconMode);
  m_textureGrid->setIconSize(QSize{m_thumbnailSize, m_thumbnailSize});
  m_textureGrid->setGridSize(QSize{m_thumbnailSize + 24, m_thumbnailSize + 36});
  m_textureGrid->setResizeMode(QListView::Adjust);
  m_textureGrid->setFlow(QListView::LeftToRight);
  m_textureGrid->setWrapping(true);
  m_textureGrid->setMovement(QListView::Static);
  m_textureGrid->setSelectionMode(QAbstractItemView::SingleSelection);
  m_textureGrid->setSpacing(4);
  m_textureGrid->setWordWrap(true);
  m_textureGrid->setTextElideMode(Qt::ElideMiddle);
  m_textureGrid->setUniformItemSizes(false);
  // Dark background matching TrenchBroom style
  m_textureGrid->setStyleSheet(
    "QListWidget { background-color: #2d2d30; color: #cccccc; }"
    "QListWidget::item:selected { background-color: #0078d4; color: white; }"
    "QListWidget::item:hover { background-color: #3e3e42; }");

  // --- Status bar ---
  m_statusLabel = new QLabel{};
  m_statusLabel->setStyleSheet("QLabel { color: #888888; padding: 2px; }");

  // --- Main layout ---
  auto* mainLayout = new QVBoxLayout{};
  mainLayout->setContentsMargins(
    LayoutConstants::NarrowHMargin,
    LayoutConstants::NarrowVMargin,
    LayoutConstants::NarrowHMargin,
    LayoutConstants::NarrowVMargin);
  mainLayout->setSpacing(LayoutConstants::NarrowVMargin);
  mainLayout->addLayout(dirLayout, 0);
  mainLayout->addLayout(filterLayout, 0);
  mainLayout->addWidget(m_textureGrid, 1);
  mainLayout->addWidget(m_statusLabel, 0);
  setLayout(mainLayout);

  // --- Connections ---
  connect(
    m_directoryCombo,
    QOverload<int>::of(&QComboBox::activated),
    this,
    &TextureBrowserPanel::onDirectoryChanged);
  connect(
    m_browseButton, &QPushButton::clicked, this, &TextureBrowserPanel::onBrowseDirectory);
  connect(m_refreshButton, &QPushButton::clicked, this, &TextureBrowserPanel::onRefresh);
  connect(
    m_filterBox, &QLineEdit::textChanged, this, &TextureBrowserPanel::onFilterChanged);
  connect(
    m_iconSizeSlider,
    &QSlider::valueChanged,
    this,
    &TextureBrowserPanel::onIconSizeChanged);
  connect(
    m_textureGrid,
    &QListWidget::itemClicked,
    this,
    &TextureBrowserPanel::onItemClicked);
  connect(
    m_textureGrid,
    &QListWidget::itemDoubleClicked,
    this,
    &TextureBrowserPanel::onItemDoubleClicked);
}

// ---------------------------------------------------------------------------
// Observer hookup
// ---------------------------------------------------------------------------

void TextureBrowserPanel::connectObservers()
{
  m_notifierConnection +=
    m_document.documentWasLoadedNotifier.connect([&] { populateDirectoryCombo(); reloadGrid(); });
  m_notifierConnection +=
    m_document.documentDidChangeNotifier.connect([&] { populateDirectoryCombo(); });

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect([&](const auto& path) {
      if (path == pref(m_document.map().gameInfo().gamePathPreference))
      {
        populateDirectoryCombo();
        reloadGrid();
      }
    });
}

// ---------------------------------------------------------------------------
// Directory combo population
// ---------------------------------------------------------------------------

void TextureBrowserPanel::populateDirectoryCombo()
{
  const auto currentText = m_directoryCombo->currentText();
  m_directoryCombo->blockSignals(true);
  m_directoryCombo->clear();

  // First entry: show all
  m_directoryCombo->addItem(
    tr("All Collections"), QVariant{QString{"__all__"}});

  // Add each material collection as a directory entry
  const auto& collections = m_document.map().materialManager().collections();
  for (const auto& collection : collections)
  {
    const auto pathStr = QString::fromStdString(collection.path().string());
    const auto displayName = pathStr.isEmpty() ? tr("(root)") : pathStr;
    m_directoryCombo->addItem(displayName, QVariant{pathStr});
  }

  // Separator before external dirs
  if (!m_externalDirectories.empty())
  {
    m_directoryCombo->insertSeparator(m_directoryCombo->count());
    for (const auto& extDir : m_externalDirectories)
    {
      const auto pathStr = QString::fromStdString(extDir.string());
      m_directoryCombo->addItem(
        tr("[External] %1").arg(pathStr), QVariant{pathStr});
    }
  }

  // Restore previous selection if possible
  const auto idx = m_directoryCombo->findText(currentText);
  m_directoryCombo->setCurrentIndex(idx >= 0 ? idx : 0);

  m_directoryCombo->blockSignals(false);
}

// ---------------------------------------------------------------------------
// Slot handlers
// ---------------------------------------------------------------------------

void TextureBrowserPanel::onDirectoryChanged(int /*index*/)
{
  reloadGrid();
}

void TextureBrowserPanel::onBrowseDirectory()
{
  const auto dir = QFileDialog::getExistingDirectory(
    this, tr("Select Texture Directory"), QString{}, QFileDialog::ShowDirsOnly);

  if (dir.isEmpty())
  {
    return;
  }

  const auto dirPath = std::filesystem::path{dir.toStdString()};

  // Don't add duplicates
  const auto it =
    std::ranges::find(m_externalDirectories, dirPath);
  if (it == m_externalDirectories.end())
  {
    m_externalDirectories.push_back(dirPath);
  }

  populateDirectoryCombo();

  // Select the newly-added directory
  for (int i = 0; i < m_directoryCombo->count(); ++i)
  {
    if (m_directoryCombo->itemData(i).toString() == dir)
    {
      m_directoryCombo->setCurrentIndex(i);
      break;
    }
  }

  reloadGrid();
}

void TextureBrowserPanel::onRefresh()
{
  reloadGrid();
}

void TextureBrowserPanel::onFilterChanged(const QString& /*text*/)
{
  applyFilter();
}

void TextureBrowserPanel::onIconSizeChanged(const int value)
{
  m_thumbnailSize = value;
  m_textureGrid->setIconSize(QSize{m_thumbnailSize, m_thumbnailSize});
  m_textureGrid->setGridSize(QSize{m_thumbnailSize + 24, m_thumbnailSize + 36});
  // Full reload to regenerate thumbnails at the new size
  reloadGrid();
}

void TextureBrowserPanel::onItemClicked(QListWidgetItem* item)
{
  if (!item)
  {
    return;
  }

  const auto materialName = item->data(Qt::UserRole).toString();
  const auto* material = m_document.map().materialManager().material(
    materialName.toStdString());

  emit materialSelected(material, materialName);
}

void TextureBrowserPanel::onItemDoubleClicked(QListWidgetItem* item)
{
  // Double-click does the same as single click for now; the signal receiver can decide
  // whether to apply the texture to the selected faces.
  onItemClicked(item);
}

// ---------------------------------------------------------------------------
// Grid loading
// ---------------------------------------------------------------------------

void TextureBrowserPanel::reloadGrid()
{
  m_textureGrid->clear();
  m_itemMaterialNames.clear();

  const auto data = m_directoryCombo->currentData().toString();

  if (data == "__all__")
  {
    loadAllCollections();
  }
  else
  {
    // Check if it's an external directory
    const auto path = std::filesystem::path{data.toStdString()};
    const auto isExternal =
      std::ranges::find(m_externalDirectories, path) != m_externalDirectories.end();

    if (isExternal)
    {
      loadFromExternalDirectory(path);
    }
    else
    {
      loadFromCollection(path);
    }
  }

  applyFilter();
  updateStatusLabel();
}

void TextureBrowserPanel::loadFromCollection(const std::filesystem::path& collectionPath)
{
  const auto& collections = m_document.map().materialManager().collections();

  for (const auto& collection : collections)
  {
    if (collection.path() == collectionPath)
    {
      for (const auto& material : collection.materials())
      {
        const auto name = QString::fromStdString(material.name());
        const auto displayName = QString::fromStdString(
          std::filesystem::path{material.name()}.filename().string());

        // Try to create a thumbnail from the absolute path on disk
        auto pixmap = QPixmap{};
        if (!material.absolutePath().empty())
        {
          pixmap = createThumbnail(material.absolutePath(), m_thumbnailSize);
        }
        if (pixmap.isNull())
        {
          pixmap = createPlaceholderThumbnail(m_thumbnailSize);
        }

        auto* item = new QListWidgetItem{QIcon{pixmap}, displayName};
        item->setData(Qt::UserRole, name);  // Store the material name
        item->setToolTip(
          name + "\n"
          + (material.absolutePath().empty()
               ? tr("(no file path)")
               : QString::fromStdString(material.absolutePath().string())));

        m_textureGrid->addItem(item);
      }
      break;
    }
  }
}

void TextureBrowserPanel::loadAllCollections()
{
  const auto enabledCollections = mdl::enabledMaterialCollections(m_document.map());
  const auto& collections = m_document.map().materialManager().collections();

  for (const auto& collection : collections)
  {
    if (
      std::ranges::find(enabledCollections, collection.path())
      == enabledCollections.end())
    {
      continue;
    }

    for (const auto& material : collection.materials())
    {
      const auto name = QString::fromStdString(material.name());
      const auto displayName = QString::fromStdString(
        std::filesystem::path{material.name()}.filename().string());

      auto pixmap = QPixmap{};
      if (!material.absolutePath().empty())
      {
        pixmap = createThumbnail(material.absolutePath(), m_thumbnailSize);
      }
      if (pixmap.isNull())
      {
        pixmap = createPlaceholderThumbnail(m_thumbnailSize);
      }

      auto* item = new QListWidgetItem{QIcon{pixmap}, displayName};
      item->setData(Qt::UserRole, name);
      item->setToolTip(
        name + "\n"
        + (material.absolutePath().empty()
             ? tr("(no file path)")
             : QString::fromStdString(material.absolutePath().string())));

      m_textureGrid->addItem(item);
    }
  }
}

void TextureBrowserPanel::loadFromExternalDirectory(
  const std::filesystem::path& dirPath)
{
  if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath))
  {
    m_statusLabel->setText(tr("Directory not found: %1")
                             .arg(QString::fromStdString(dirPath.string())));
    return;
  }

  auto entries = std::vector<std::filesystem::path>{};
  try
  {
    for (const auto& entry : std::filesystem::recursive_directory_iterator{
           dirPath, std::filesystem::directory_options::skip_permission_denied})
    {
      if (!entry.is_regular_file())
      {
        continue;
      }
      const auto ext = entry.path().extension().string();
      if (isSupportedExtension(ext))
      {
        entries.push_back(entry.path());
      }
    }
  }
  catch (const std::filesystem::filesystem_error& e)
  {
    m_statusLabel->setText(
      tr("Error scanning directory: %1").arg(QString::fromUtf8(e.what())));
    return;
  }

  // Sort by filename
  std::ranges::sort(entries, [](const auto& a, const auto& b) {
    return a.filename() < b.filename();
  });

  for (const auto& filePath : entries)
  {
    const auto fileName = QString::fromStdString(filePath.stem().string());

    auto pixmap = createThumbnail(filePath, m_thumbnailSize);
    if (pixmap.isNull())
    {
      pixmap = createPlaceholderThumbnail(m_thumbnailSize);
    }

    // For external textures, the "material name" is the filename stem.
    // The user can configure the game to resolve this later.
    auto* item = new QListWidgetItem{QIcon{pixmap}, fileName};
    item->setData(Qt::UserRole, fileName);
    item->setToolTip(
      fileName + "\n" + QString::fromStdString(filePath.string()));

    m_textureGrid->addItem(item);
  }
}

// ---------------------------------------------------------------------------
// Filtering
// ---------------------------------------------------------------------------

void TextureBrowserPanel::applyFilter()
{
  const auto filterText = m_filterBox->text().trimmed().toLower();

  int visibleCount = 0;
  for (int i = 0; i < m_textureGrid->count(); ++i)
  {
    auto* item = m_textureGrid->item(i);
    if (filterText.isEmpty())
    {
      item->setHidden(false);
      ++visibleCount;
    }
    else
    {
      const auto name = item->data(Qt::UserRole).toString().toLower();
      const auto displayName = item->text().toLower();
      const auto matches =
        name.contains(filterText) || displayName.contains(filterText);
      item->setHidden(!matches);
      if (matches)
      {
        ++visibleCount;
      }
    }
  }

  updateStatusLabel();
}

// ---------------------------------------------------------------------------
// Status
// ---------------------------------------------------------------------------

void TextureBrowserPanel::updateStatusLabel()
{
  int total = m_textureGrid->count();
  int visible = 0;
  for (int i = 0; i < total; ++i)
  {
    if (!m_textureGrid->item(i)->isHidden())
    {
      ++visible;
    }
  }

  if (visible == total)
  {
    m_statusLabel->setText(tr("%1 textures").arg(total));
  }
  else
  {
    m_statusLabel->setText(tr("%1 of %2 textures shown").arg(visible).arg(total));
  }
}

// ---------------------------------------------------------------------------
// Thumbnail helpers
// ---------------------------------------------------------------------------

QPixmap TextureBrowserPanel::createThumbnail(
  const std::filesystem::path& filePath, const int thumbnailSize)
{
  const auto qPath = QString::fromStdString(filePath.string());
  auto image = QImage{qPath};
  if (image.isNull())
  {
    return {};
  }

  // Scale to fit the thumbnail size, preserving aspect ratio
  return QPixmap::fromImage(
    image.scaled(thumbnailSize, thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QPixmap TextureBrowserPanel::createPlaceholderThumbnail(const int thumbnailSize)
{
  auto pixmap = QPixmap{thumbnailSize, thumbnailSize};
  pixmap.fill(QColor{60, 60, 65});

  auto painter = QPainter{&pixmap};
  painter.setPen(QColor{100, 100, 105});
  painter.drawRect(0, 0, thumbnailSize - 1, thumbnailSize - 1);

  // Draw a "?" in the center
  auto font = painter.font();
  font.setPixelSize(thumbnailSize / 3);
  painter.setFont(font);
  painter.setPen(QColor{120, 120, 125});
  painter.drawText(pixmap.rect(), Qt::AlignCenter, "?");
  painter.end();

  return pixmap;
}

bool TextureBrowserPanel::isSupportedExtension(const std::string& ext)
{
  const auto lower = QString::fromStdString(ext).toLower().toStdString();
  return std::ranges::find(s_supportedExtensions, lower) != s_supportedExtensions.end();
}

} // namespace tb::ui
