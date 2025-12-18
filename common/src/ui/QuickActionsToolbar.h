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

#include <QToolBar>

class QAction;
class QToolButton;

namespace tb::ui
{
class MapDocument;
class MapFrame;

/**
 * Quick actions toolbar with common operations for fast access
 */
class QuickActionsToolbar : public QToolBar
{
  Q_OBJECT
private:
  MapFrame& m_frame;
  MapDocument& m_document;

  // CSG actions
  QAction* m_csgUnion = nullptr;
  QAction* m_csgSubtract = nullptr;
  QAction* m_csgIntersect = nullptr;
  QAction* m_csgHollow = nullptr;

  // Transform actions
  QAction* m_flipH = nullptr;
  QAction* m_flipV = nullptr;
  QAction* m_rotateLeft = nullptr;
  QAction* m_rotateRight = nullptr;

  // View actions
  QAction* m_toggleGrid = nullptr;
  QAction* m_toggleTextures = nullptr;
  
  // Snap actions
  QToolButton* m_gridSnapButton = nullptr;

public:
  QuickActionsToolbar(MapFrame& frame, MapDocument& document, QWidget* parent = nullptr);

private:
  void createActions();
  void createCsgSection();
  void createTransformSection();
  void createViewSection();
  void updateActionStates();
};

} // namespace tb::ui
