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

#pragma once

#include <QDockWidget>

#include "NotifierConnection.h"

class QListWidget;
class QPushButton;

namespace tb::ui
{
class MapDocument;

/**
 * Dock widget that displays undo/redo history with the ability to 
 * jump to specific points in the command history.
 */
class UndoHistoryPanel : public QDockWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;
  NotifierConnection m_notifierConnection;
  QListWidget* m_historyList = nullptr;
  QPushButton* m_undoButton = nullptr;
  QPushButton* m_redoButton = nullptr;

public:
  explicit UndoHistoryPanel(MapDocument& document, QWidget* parent = nullptr);

private:
  void createGui();
  void updateHistoryList();
  void connectObservers();

  void onUndoClicked();
  void onRedoClicked();
  void onHistoryItemDoubleClicked(int row);
};

} // namespace tb::ui
