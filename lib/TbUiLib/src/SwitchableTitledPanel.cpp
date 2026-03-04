/*
 Copyright (C) 2024 Kristian Duske

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

#include "ui/SwitchableTitledPanel.h"

#include <QBoxLayout>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QStackedLayout>

#include <vector>

#include "ui/BorderLine.h"
#include "ui/ClickableTitleBar.h"

#include "kd/contracts.h"

namespace tb::ui
{

SwitchableTitledPanel::SwitchableTitledPanel(
  const QString& title, std::initializer_list<QString> stateTexts, QWidget* parent)
  : QWidget{parent}
{
  init(title, stateTexts);
}

void SwitchableTitledPanel::init(
  const QString& title, std::initializer_list<QString> stateTexts)
{
  const auto texts = std::vector<QString>{stateTexts};
  contract_pre(texts.size() >= 2);

  m_titleBar = new ClickableTitleBar{title, texts[1]};
  m_divider = new BorderLine{};
  m_stackedLayout = new QStackedLayout{};

  m_panels.reserve(texts.size());
  for (size_t i = 0; i < texts.size(); ++i)
  {
    // The state text shown on each panel tells the user what the *next* panel is.
    // For panel i, the state text is the name of panel (i+1) % N.
    const auto nextIndex = (i + 1) % texts.size();
    m_panels.push_back({new QWidget{}, texts[nextIndex]});
    m_stackedLayout->addWidget(m_panels.back().panel);
  }

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(0);
  outerLayout->addWidget(m_titleBar, 0);
  outerLayout->addWidget(m_divider, 0);
  outerLayout->addLayout(m_stackedLayout, 1);
  setLayout(outerLayout);

  connect(m_titleBar, &ClickableTitleBar::titleBarClicked, this, [&]() {
    setCurrentIndex((currentIndex() + 1) % m_panels.size());
  });
}

QWidget* SwitchableTitledPanel::getPanel(const size_t index) const
{
  contract_pre(index < m_panels.size());

  return m_panels[index].panel;
}

size_t SwitchableTitledPanel::panelCount() const
{
  return m_panels.size();
}

size_t SwitchableTitledPanel::currentIndex() const
{
  return size_t(m_stackedLayout->currentIndex());
}

void SwitchableTitledPanel::setCurrentIndex(const size_t index)
{
  m_stackedLayout->setCurrentIndex(int(index));
  m_titleBar->setStateText(m_panels[index].stateText);
}

QByteArray SwitchableTitledPanel::saveState() const
{
  auto result = QByteArray{};
  auto stream = QDataStream{&result, QIODevice::WriteOnly};
  stream << int(currentIndex());
  return result;
}

bool SwitchableTitledPanel::restoreState(const QByteArray& state)
{
  auto stream = QDataStream{state};
  int currentIndex;
  stream >> currentIndex;

  if (
    stream.status() == QDataStream::Ok && currentIndex >= 0
    && currentIndex < int(m_panels.size()))
  {
    setCurrentIndex(size_t(currentIndex));
    return true;
  }

  return false;
}

} // namespace tb::ui
