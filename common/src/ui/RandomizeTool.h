#pragma once

#include "ui/Tool.h"

#include "vm/vec.h"

namespace tb
{
namespace mdl
{
class Grid;
}

namespace ui
{
class MapDocument;

class RandomizeTool : public Tool
{
private:
  MapDocument& m_document;

public:
  explicit RandomizeTool(MapDocument& document);

  bool doActivate() override;

  const mdl::Grid& grid() const;

  void applyRandomization(
    const vm::vec3d& minTranslate,
    const vm::vec3d& maxTranslate,
    const vm::vec3d& minRotate,
    const vm::vec3d& maxRotate,
    const vm::vec3d& minScale,
    const vm::vec3d& maxScale);

private:
  QWidget* doCreatePage(QWidget* parent) override;
};

} // namespace ui
} // namespace tb
