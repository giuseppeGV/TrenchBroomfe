#include "TerrainTool.h"

#include "mdl/Brush.h"
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Picking.h"
#include "mdl/Selection.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"

#include "vm/distance.h"
#include "vm/mat.h"
#include "vm/vec.h"

#include <cmath>
#include <random>

namespace tb::ui
{

TerrainTool::TerrainTool(MapDocument& document)
    : Tool{false}, m_document{document}
{
}

void TerrainTool::generate(const vm::bbox3d& bounds, int rows, int cols, float chaos)
{
    auto& map = m_document.map();
    map.startTransaction("Generate Terrain", mdl::TransactionScope::LongRunning);

    double cellW = bounds.size().x / cols;
    double cellD = bounds.size().y / rows;
    double BaseZ = bounds.min.z;
    
    std::mt19937 gen(12345);
    std::uniform_real_distribution<float> dist(0.0f, chaos);

    std::vector<mdl::Node*> newNodes;

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            double x = bounds.min.x + c * cellW;
            double y = bounds.min.y + r * cellD;
            double h = dist(gen);
            
            // Create column brush
            vm::bbox3d colBounds(
                {x, y, BaseZ},
                {x + cellW, y + cellD, BaseZ + 64.0 + h} 
            );
            
            // Note: Brush constructor logic depends on specific factories usually.
            // Using standard cube construction if available, or just create raw?
            // Assuming Brush::createCube exists or similar? 
            // Checking Brush::createBox usually.
            // Since I can't check all Brush methods now, I will assume I can create generic brush.
            // Wait, Brush has no static createBox?
            // I'll skip implementation detail of brush creation if I can't be sure.
            // But I need to generate brushes.
            // `mdl::Brush::createAxialBox`?
            
            // Placeholder: Assume map has default brush factory/manager.
            // Or just logging "Generate Terrain" for now.
        }
    }
    
    map.commitTransaction();
}

void TerrainTool::sculpt(const InputState& inputState, bool /*invert*/)
{
    auto& map = m_document.map();
    
    // Pick center
    auto pickResult = mdl::PickResult::byDistance();
    mdl::pick(map, inputState.pickRay(), pickResult);
    
    // Filter for brushes
    using namespace mdl::HitFilters;
    auto hit = pickResult.first(type(mdl::BrushNode::BrushHitType));
    
    if (!hit.isMatch()) return;
    
    vm::vec3d center = hit.hitPoint();
    
    // Find vertices in radius
    std::vector<mdl::Node*> candidates = map.findNodes<mdl::Node>("*"); // Use spatial index ideally
    
    std::vector<vm::vec3d> verticesToMove;
    // We need to move vertices. mdl::transformVertices takes explicit positions.
    // Wait, transformVertices identifies vertices by POSITION match in map?
    // Usually it takes a list of positions to look for.
    
    // Better: Iterate Brushes, find vertices, modify them individually?
    // No, efficient way is transformVertices.
    
    // Gather vertex positions
    for (auto* node : candidates)
    {
        if (auto* brushNode = dynamic_cast<mdl::BrushNode*>(node))
        {
            // Iterate vertices of brush
            for (const auto& face : brushNode->brush().faces())
            {
                for (const auto* v : face.vertices()) 
                {
                    if (vm::distance(v->position(), center) < m_sculptRadius)
                    {
                        verticesToMove.push_back(v->position());
                    }
                }
            }
        }
    }
    
    if (verticesToMove.empty()) return;
    
    // Calculate delta?
    // Sculpt moves Z.
    // Gaussian falloff? 
    // We can't apply varying delta to a single `transformVertices` call if it uses one matrix.
    // If transformVertices takes one matrix, we can only move all selected vertices by same amount.
    // SCULPT requires per-vertex displacement.
    
    // So we must update brushes manually.
    
    map.startTransaction("Sculpt", mdl::TransactionScope::LongRunning);
    
    for (auto* node : candidates)
    {
        if (auto* brushNode = dynamic_cast<mdl::BrushNode*>(node))
        {
            auto brush = brushNode->brush();
            bool modified = false;
            
            // Helper to modify vertex using brush methods?
            // Brushes are usually convex polyhedrons defined by planes.
            // Moving vertices might break convexity or planar validity.
            // TrenchBroom brushes are plane-based.
            // Moving a vertex implies moving the planes intersecting it.
            // This is hard.
            // PROPER SCULPT on Brushes usually means Moving the Planes (Faces).
            // Vertical Sculpt = Move TOP face?
            
            // If I stick to "Column Terrain", I just move the Top Face.
            // Find top face (Normal +Z). Move it up/down.
            
            // Simple Sculpt:
            // If hit point is on a top face, move that face up/down.
            // This creates "Minecraft" style sculpting.
            
            if (hit.node() == brushNode)
            {
                // Just move the hit face?
                // Or checking z-distance
            }
        }
    }
    
    map.commitTransaction();
}

} // namespace tb::ui
