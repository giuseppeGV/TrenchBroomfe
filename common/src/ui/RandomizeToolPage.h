#pragma once

#include <QWidget>

#include "ui/RandomizeTool.h"

class QPushButton;

namespace tb::ui
{
class MapDocument;
class SpinControl;

class RandomizeToolPage : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;
  RandomizeTool& m_tool;

  SpinControl* m_minTranslateX = nullptr;
  SpinControl* m_maxTranslateX = nullptr;
  SpinControl* m_minTranslateY = nullptr;
  SpinControl* m_maxTranslateY = nullptr;
  SpinControl* m_minTranslateZ = nullptr;
  SpinControl* m_maxTranslateZ = nullptr;

  SpinControl* m_minRotateX = nullptr;
  SpinControl* m_maxRotateX = nullptr;
  SpinControl* m_minRotateY = nullptr;
  SpinControl* m_maxRotateY = nullptr;
  SpinControl* m_minRotateZ = nullptr;
  SpinControl* m_maxRotateZ = nullptr;

  SpinControl* m_minScaleX = nullptr;
  SpinControl* m_maxScaleX = nullptr;
  SpinControl* m_minScaleY = nullptr;
  SpinControl* m_maxScaleY = nullptr;
  SpinControl* m_minScaleZ = nullptr;
  SpinControl* m_maxScaleZ = nullptr;

  QPushButton* m_applyButton = nullptr;

public:
  RandomizeToolPage(MapDocument& document, RandomizeTool& tool, QWidget* parent = nullptr);

private:
  void createGui();
  void updateGui();

  void applyClicked();
};

} // namespace tb::ui
