#include "SymmetryManager.h"

namespace tb::mdl
{

vm::vec3d SymmetryManager::reflect(const vm::vec3d& point) const
{
    if (!m_enabled) return point;

    vm::vec3d local = point - m_origin;
    vm::vec3d reflected = local;

    switch (m_axis)
    {
    case SymmetryAxis::X: reflected[0] = -reflected[0]; break;
    case SymmetryAxis::Y: reflected[1] = -reflected[1]; break;
    case SymmetryAxis::Z: reflected[2] = -reflected[2]; break;
    case SymmetryAxis::Custom:
        // Not implemented yet
        break;
    }

    return reflected + m_origin;
}

vm::vec3d SymmetryManager::reflectVector(const vm::vec3d& vec) const
{
    if (!m_enabled) return vec;

    vm::vec3d reflected = vec;

    switch (m_axis)
    {
    case SymmetryAxis::X: reflected[0] = -reflected[0]; break;
    case SymmetryAxis::Y: reflected[1] = -reflected[1]; break;
    case SymmetryAxis::Z: reflected[2] = -reflected[2]; break;
    case SymmetryAxis::Custom:
        break;
    }

    return reflected;
}

} // namespace tb::mdl
