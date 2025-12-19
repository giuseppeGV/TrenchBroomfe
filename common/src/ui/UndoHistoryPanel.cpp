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

#include "UndoHistoryPanel.h"

#include "mdl/CommandProcessor.h"
#include "mdl/Map.h"
#include "ui/MapDocument.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace tb::ui
{

UndoHistoryPanel::UndoHistoryPanel(MapDocument& document, QWidget* parent)
  : QDockWidget{tr("Undo History"), parent}
  , m_document{document}
{
  setObjectName("UndoHistoryPanel");
  setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  
  createGui();
  connectObservers();
  updateHistoryList();
}

void UndoHistoryPanel::createGui()
{
  auto* mainWidget = new QWidget{};
  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);

  // History list
  m_historyList = new QListWidget{};
  m_historyList->setAlternatingRowColors(true);
  m_historyList->setSelectionMode(QAbstractItemView::SingleSelection);
  
  connect(m_historyList, &QListWidget::itemDoubleClicked, 
          this, [this]() { onHistoryItemDoubleClicked(m_historyList->currentRow()); });
  
  layout->addWidget(m_historyList, 1);

  // Button row
  auto* buttonLayout = new QHBoxLayout{};
  
  m_undoButton = new QPushButton{tr("Undo")};
  m_undoButton->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
  connect(m_undoButton, &QPushButton::clicked, this, &UndoHistoryPanel::onUndoClicked);
  
  m_redoButton = new QPushButton{tr("Redo")};
  m_redoButton->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
  connect(m_redoButton, &QPushButton::clicked, this, &UndoHistoryPanel::onRedoClicked);
  
  buttonLayout->addWidget(m_undoButton);
  buttonLayout->addWidget(m_redoButton);
  buttonLayout->addStretch();
  
  layout->addLayout(buttonLayout);

  mainWidget->setLayout(layout);
  setWidget(mainWidget);
}

void UndoHistoryPanel::updateHistoryList()
{
  m_historyList->clear();
  
  auto& map = m_document.map();
  
  // Get command names from the undo stack
  const auto& undoStack = map.commandProcessor().undoStack();
  const auto& redoStack = map.commandProcessor().redoStack();
  
  // Add undo items (completed commands, displayed bottom to top)
  for (size_t i = undoStack.size(); i > 0; --i)
  {
    auto* item = new QListWidgetItem{QString::fromStdString(undoStack[i - 1]->name())};
    item->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    item->setForeground(Qt::white);
    m_historyList->addItem(item);
  }
  
  // Mark the current position
  auto* currentItem = new QListWidgetItem{tr("--- Current State ---")};
  currentItem->setBackground(QColor("#45475a"));
  currentItem->setForeground(QColor("#89b4fa"));
  currentItem->setFlags(Qt::NoItemFlags);
  m_historyList->addItem(currentItem);
  
  // Add redo items (undone commands, top to bottom)
  for (size_t i = redoStack.size(); i > 0; --i)
  {
    auto* item = new QListWidgetItem{QString::fromStdString(redoStack[i - 1]->name())};
    item->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    item->setForeground(QColor("#6c7086")); // Dimmed
    m_historyList->addItem(item);
  }
  
  // Update button states
  m_undoButton->setEnabled(map.commandProcessor().canUndo());
  m_redoButton->setEnabled(map.commandProcessor().canRedo());
  
  // Scroll to current position
  m_historyList->scrollToItem(currentItem);
}

void UndoHistoryPanel::connectObservers()
{
  // Connect to document notifications
  m_notifierConnection = m_document.modificationStateDidChangeNotifier.connect(
      this, &UndoHistoryPanel::updateHistoryList);
}

void UndoHistoryPanel::onUndoClicked()
{
  if (m_document.map().commandProcessor().canUndo())
  {
    m_document.map().commandProcessor().undo();
    updateHistoryList();
  }
}

void UndoHistoryPanel::onRedoClicked()
{
  if (m_document.map().commandProcessor().canRedo())
  {
    m_document.map().commandProcessor().redo();
    updateHistoryList();
  }
}

void UndoHistoryPanel::onHistoryItemDoubleClicked(int row)
{
  auto& map = m_document.map();
  const auto undoCount = static_cast<int>(map.commandProcessor().undoStack().size());
  
  // Row 0 to undoCount-1 are undo items (in reverse order)
  // Row undoCount is the "current state" marker
  // Row undoCount+1 onwards are redo items
  
  if (row < undoCount)
  {
    // Need to undo (undoCount - row) times
    const int timesToUndo = undoCount - row;
    for (int i = 0; i < timesToUndo && map.commandProcessor().canUndo(); ++i)
    {
      map.commandProcessor().undo();
    }
  }
  else if (row > undoCount)
  {
    // Need to redo (row - undoCount) times
    const int timesToRedo = row - undoCount;
    for (int i = 0; i < timesToRedo && map.commandProcessor().canRedo(); ++i)
    {
      map.commandProcessor().redo();
    }
  }
  
  updateHistoryList();
}

} // namespace tb::ui
