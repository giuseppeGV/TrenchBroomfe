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

#include "QuickActionsToolbar.h"

#include "io/ResourceUtils.h"
#include "mdl/Grid.h"
#include "mdl/Map.h"
#include "ui/MapDocument.h"
#include "ui/MapFrame.h"

#include <QAction>
#include <QMenu>
#include <QToolButton>

namespace tb::ui
{

QuickActionsToolbar::QuickActionsToolbar(MapFrame& frame, MapDocument& document, QWidget* parent)
  : QToolBar{tr("Quick Actions"), parent}
  , m_frame{frame}
  , m_document{document}
{
  setObjectName("QuickActionsToolbar");
  setMovable(true);
  setFloatable(true);
  
  createActions();
  
  connect(&m_document, &MapDocument::modificationStateDidChangeNotifier,
          this, &QuickActionsToolbar::updateActionStates);
}

void QuickActionsToolbar::createActions()
{
  createCsgSection();
  addSeparator();
  createTransformSection();
  addSeparator();
  createViewSection();
  
  updateActionStates();
}

void QuickActionsToolbar::createCsgSection()
{
  // CSG Union
  m_csgUnion = addAction(io::loadSVGIcon("CSGUnion.svg"), tr("CSG Union"));
  m_csgUnion->setToolTip(tr("CSG Union (Ctrl+Shift+U)"));
  m_csgUnion->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U));
  connect(m_csgUnion, &QAction::triggered, &m_frame, &MapFrame::csgUnion);

  // CSG Subtract
  m_csgSubtract = addAction(io::loadSVGIcon("CSGSubtract.svg"), tr("CSG Subtract"));
  m_csgSubtract->setToolTip(tr("CSG Subtract (Ctrl+Shift+S)"));
  connect(m_csgSubtract, &QAction::triggered, &m_frame, &MapFrame::csgSubtract);

  // CSG Intersect
  m_csgIntersect = addAction(io::loadSVGIcon("CSGIntersect.svg"), tr("CSG Intersect"));
  m_csgIntersect->setToolTip(tr("CSG Intersect (Ctrl+Shift+I)"));
  connect(m_csgIntersect, &QAction::triggered, &m_frame, &MapFrame::csgIntersect);

  // CSG Hollow
  m_csgHollow = addAction(io::loadSVGIcon("CSGHollow.svg"), tr("CSG Hollow"));
  m_csgHollow->setToolTip(tr("CSG Hollow (Ctrl+Shift+H)"));
  connect(m_csgHollow, &QAction::triggered, &m_frame, &MapFrame::csgHollow);
}

void QuickActionsToolbar::createTransformSection()
{
  // Flip Horizontal
  m_flipH = addAction(io::loadSVGIcon("FlipHorizontal.svg"), tr("Flip Horizontal"));
  m_flipH->setToolTip(tr("Flip Horizontal (Ctrl+F)"));
  connect(m_flipH, &QAction::triggered, [this]() {
      if (auto* action = m_frame.findAction("Controls/Map view/Flip objects horizontally"))
          action->trigger();
  });

  // Flip Vertical
  m_flipV = addAction(io::loadSVGIcon("FlipVertical.svg"), tr("Flip Vertical"));
  m_flipV->setToolTip(tr("Flip Vertical (Ctrl+Alt+F)"));
  connect(m_flipV, &QAction::triggered, [this]() {
      if (auto* action = m_frame.findAction("Controls/Map view/Flip objects vertically"))
          action->trigger();
  });

  // Rotate Left
  m_rotateLeft = addAction(io::loadSVGIcon("RotateLeft.svg"), tr("Rotate Left"));
  m_rotateLeft->setToolTip(tr("Rotate 90° Left"));
  connect(m_rotateLeft, &QAction::triggered, [this]() {
      if (auto* action = m_frame.findAction("Controls/Map view/Yaw objects clockwise"))
          action->trigger();
  });

  // Rotate Right
  m_rotateRight = addAction(io::loadSVGIcon("RotateRight.svg"), tr("Rotate Right"));
  m_rotateRight->setToolTip(tr("Rotate 90° Right"));
  connect(m_rotateRight, &QAction::triggered, [this]() {
      if (auto* action = m_frame.findAction("Controls/Map view/Yaw objects counter-clockwise"))
          action->trigger();
  });
}

void QuickActionsToolbar::createViewSection()
{
  // Toggle Grid
  m_toggleGrid = addAction(io::loadSVGIcon("GridToggle.svg"), tr("Toggle Grid"));
  m_toggleGrid->setToolTip(tr("Toggle Grid Visibility"));
  m_toggleGrid->setCheckable(true);
  connect(m_toggleGrid, &QAction::triggered, [this]() {
        // Toggle directly on the map grid object, safer than relying on frame action existence
        m_document.map().grid().setVisible(!m_document.map().grid().visible());
  });

  // Toggle Textures
  m_toggleTextures = addAction(io::loadSVGIcon("TextureToggle.svg"), tr("Toggle Textures"));
  m_toggleTextures->setToolTip(tr("Toggle Material Visibility"));
  m_toggleTextures->setCheckable(true);
  connect(m_toggleTextures, &QAction::triggered, [this](bool checked) {
    if (checked) {
        if (auto* action = m_frame.findAction("Controls/Map view/View Filter > Show textures"))
            action->trigger();
    } else {
        if (auto* action = m_frame.findAction("Controls/Map view/View Filter > Hide textures"))
            action->trigger();
    }
  });

  // Grid snap dropdown
  auto* snapMenu = new QMenu{this};
  snapMenu->addAction(tr("1 unit"), [this]() { m_document.map().grid().setSize(0); });
  snapMenu->addAction(tr("2 units"), [this]() { m_document.map().grid().setSize(1); });
  snapMenu->addAction(tr("4 units"), [this]() { m_document.map().grid().setSize(2); });
  snapMenu->addAction(tr("8 units"), [this]() { m_document.map().grid().setSize(3); });
  snapMenu->addAction(tr("16 units"), [this]() { m_document.map().grid().setSize(4); });
  snapMenu->addAction(tr("32 units"), [this]() { m_document.map().grid().setSize(5); });
  snapMenu->addAction(tr("64 units"), [this]() { m_document.map().grid().setSize(6); });
  snapMenu->addAction(tr("128 units"), [this]() { m_document.map().grid().setSize(7); });
  
  m_gridSnapButton = new QToolButton{this};
  m_gridSnapButton->setText(tr("Grid"));
  m_gridSnapButton->setToolTip(tr("Grid Size"));
  m_gridSnapButton->setPopupMode(QToolButton::InstantPopup);
  m_gridSnapButton->setMenu(snapMenu);
  addWidget(m_gridSnapButton);
}

void QuickActionsToolbar::updateActionStates()
{
  const bool hasSelection = m_document.map().selection().hasAny();
  const bool hasBrushes = m_document.map().selection().hasOnlyBrushes();
  
  m_csgUnion->setEnabled(hasBrushes);
  m_csgSubtract->setEnabled(hasBrushes);
  m_csgIntersect->setEnabled(hasBrushes);
  m_csgHollow->setEnabled(hasBrushes);
  
  m_flipH->setEnabled(hasSelection);
  m_flipV->setEnabled(hasSelection);
  m_rotateLeft->setEnabled(hasSelection);
  m_rotateRight->setEnabled(hasSelection);
  
  m_toggleGrid->setChecked(m_document.map().grid().visible());
}

} // namespace tb::ui
