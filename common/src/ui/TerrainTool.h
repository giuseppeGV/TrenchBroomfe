#pragma once

#include "ui/Tool.h"
#include "vm/bbox.h"

namespace tb::ui
{
class MapDocument;
class InputState;

class TerrainTool : public Tool
{
private:
    MapDocument& m_document;
    float m_sculptRadius = 64.0f;
    float m_sculptIntensity = 16.0f;

public:
    explicit TerrainTool(MapDocument& document);

    float sculptRadius() const { return m_sculptRadius; }
    void setSculptRadius(float radius) { m_sculptRadius = radius; }

    float sculptIntensity() const { return m_sculptIntensity; }
    void setSculptIntensity(float intensity) { m_sculptIntensity = intensity; }

    void sculpt(const InputState& inputState, bool invert);
    void generate(const vm::bbox3d& bounds, int rows, int cols, float chaos);
};

} // namespace tb::ui
