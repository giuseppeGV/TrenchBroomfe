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

#pragma once

#include <QWidget>

#include "NotifierConnection.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>


class QComboBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QSlider;
class QLabel;

namespace tb
{
namespace gl
{
class Material;
} // namespace gl

namespace ui
{
class MapDocument;

/**
 * A standalone texture browser panel that uses native Qt widgets (QListWidget in icon
 * mode) to display textures in a grid. It allows the user to:
 *   - Choose from existing material collection directories
 *   - Browse for arbitrary external directories on disk
 *   - Search/filter textures by name
 *   - Adjust the thumbnail grid size
 *   - Select a texture, emitting a signal with the corresponding material pointer
 *
 * This bypasses the virtual filesystem layer entirely for directory scanning, loading
 * thumbnails directly via QImage, which handles PNG, JPG, TGA, BMP and other common
 * formats natively.
 */
class TextureBrowserPanel : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;

  QComboBox* m_directoryCombo = nullptr;
  QPushButton* m_browseButton = nullptr;
  QPushButton* m_refreshButton = nullptr;
  QLineEdit* m_filterBox = nullptr;
  QSlider* m_iconSizeSlider = nullptr;
  QLabel* m_statusLabel = nullptr;
  QListWidget* m_textureGrid = nullptr;

  // External directories the user has added via the browse button
  std::vector<std::filesystem::path> m_externalDirectories;

  // Maps item row -> material name for items backed by MaterialManager
  std::unordered_map<int, std::string> m_itemMaterialNames;

  // Current thumbnail size in pixels
  int m_thumbnailSize = 96;

  // Supported image extensions for external directory scanning
  static const std::vector<std::string> s_supportedExtensions;

  NotifierConnection m_notifierConnection;

public:
  explicit TextureBrowserPanel(MapDocument& document, QWidget* parent = nullptr);

signals:
  /**
   * Emitted when the user clicks a texture in the grid.
   * @param material Pointer to the Material in the MaterialManager, or nullptr if the
   *                 texture is from an external directory and not loaded into the
   * manager.
   * @param materialName The material/texture name. For engine-loaded materials this is
   *                     the canonical name; for external textures it is the filename
   * stem.
   * @param filePath The absolute file path on disk, or empty if unavailable.
   */
  void materialSelected(
    const gl::Material* material, const QString& materialName, const QString& filePath);

private:
  void createGui();
  void connectObservers();

  void populateDirectoryCombo();
  void onDirectoryChanged(int index);
  void onBrowseDirectory();
  void onRefresh();
  void onFilterChanged(const QString& text);
  void onIconSizeChanged(int value);
  void onItemClicked(QListWidgetItem* item);
  void onItemDoubleClicked(QListWidgetItem* item);

  /** Reload the texture grid for the currently selected directory. */
  void reloadGrid();

  /** Populate the grid from a MaterialManager collection path. */
  void loadFromCollection(const std::filesystem::path& collectionPath);

  /** Populate the grid showing all materials from all enabled collections. */
  void loadAllCollections();

  /** Populate the grid from an external filesystem directory. */
  void loadFromExternalDirectory(const std::filesystem::path& dirPath);

  /** Apply the current filter text to the grid, hiding/showing items. */
  void applyFilter();

  /** Update the status bar label. */
  void updateStatusLabel();

  /** Create a thumbnail QPixmap for a texture file on disk. */
  static QPixmap createThumbnail(
    const std::filesystem::path& filePath, int thumbnailSize);

  /** Create a placeholder thumbnail for materials without a file path. */
  static QPixmap createPlaceholderThumbnail(int thumbnailSize);

  /** Check if a file extension is supported for thumbnail loading. */
  static bool isSupportedExtension(const std::string& ext);
};

} // namespace ui
} // namespace tb
