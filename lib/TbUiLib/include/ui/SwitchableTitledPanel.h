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

#include <array>
#include <vector>

class QStackedLayout;

namespace tb::ui
{
class BorderLine;
class ClickableTitleBar;

class SwitchableTitledPanel : public QWidget
{
  Q_OBJECT
private:
  struct SwitchablePanel
  {
    QWidget* panel = nullptr;
    QString stateText;
  };

  ClickableTitleBar* m_titleBar = nullptr;
  BorderLine* m_divider = nullptr;
  QStackedLayout* m_stackedLayout = nullptr;
  std::vector<SwitchablePanel> m_panels;

public:
  // Legacy 2-panel constructor (preserves source compatibility)
  explicit SwitchableTitledPanel(
    const QString& title,
    const std::array<QString, 2>& stateTexts,
    QWidget* parent = nullptr);

  // N-panel constructor
  explicit SwitchableTitledPanel(
    const QString& title,
    const std::vector<QString>& stateTexts,
    QWidget* parent = nullptr);

  QWidget* getPanel(size_t index) const;

  size_t panelCount() const;
  size_t currentIndex() const;
  void setCurrentIndex(size_t index);

  QByteArray saveState() const;
  bool restoreState(const QByteArray& state);

private:
  void init(const QString& title, const std::vector<QString>& stateTexts);
};

} // namespace tb::ui
