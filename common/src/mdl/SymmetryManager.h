#pragma once

#include "vm/vec.h"
#include "vm/plane.h"

#include <optional>

namespace tb::mdl
{

enum class SymmetryAxis
{
    X,
    Y,
    Z,
    Custom
};

class SymmetryManager
{
private:
    bool m_enabled = false;
    SymmetryAxis m_axis = SymmetryAxis::X;
    vm::vec3d m_origin = vm::vec3d::zero();
    vm::plane3d m_customPlane;

public:
    SymmetryManager() = default;

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

    SymmetryAxis axis() const { return m_axis; }
    void setAxis(SymmetryAxis axis) { m_axis = axis; }

    const vm::vec3d& origin() const { return m_origin; }
    void setOrigin(const vm::vec3d& origin) { m_origin = origin; }

    vm::vec3d reflect(const vm::vec3d& point) const;
    vm::vec3d reflectVector(const vm::vec3d& vec) const;
};

} // namespace tb::mdl
