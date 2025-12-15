#include "Matchers.h"
#include "TestParserStatus.h"
#include "TestUtils.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{
    TEST_CASE("Brush::bevelEdge")
    {
        const auto worldBounds = vm::bbox3d{4096.0};

        SECTION("Bevel an edge of a cube")
        {
            // Create a 64x64x64 cube centered at origin
            // Vertices at +/- 32
            auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
            auto brush = builder.createCuboid(vm::bbox3d{{-32, -32, -32}, {32, 32, 32}}, "material")
                       | kdl::value();

            REQUIRE(brush.faceCount() == 6u);
            REQUIRE(brush.edgeCount() == 12u);
            REQUIRE(brush.vertexCount() == 8u);

            // Target the vertical edge at (+32, +32)
            // It should connect (+32, +32, -32) and (+32, +32, +32)
            vm::segment3d targetEdge{ {32, 32, -32}, {32, 32, 32} };
            
            // Bevel distance 8.0
            double bevelDist = 8.0;
            
            auto result = brush.bevelEdge(worldBounds, targetEdge, bevelDist, false);
            REQUIRE(result.is_success());

            // Verification
            // The edge was shared by Right (+X) and Back (+Y) faces.
            // (Note: Directions depend on axis conventions, assuming standard +X=Right, +Y=Back/Forward).
            // Actually TrenchBroom uses +Z Up.
            // Faces at +32, +32.
            
            // We expect 1 new face.
            CHECK(brush.faceCount() == 7u);
            
            // We expect the original 2 vertices on this edge to be replaced by 4 new vertices.
            // V_new = V_old - 2 + 4 = V_old + 2
            CHECK(brush.vertexCount() == 10u);
            CHECK(brush.edgeCount() == 15u);
            
            CHECK(brush.fullySpecified());
            
            // Check bounds - should be largely same?
            // The bevel removes volume, so bounds might shrink slightly (the corner is gone).
            // The original corner was at (32, 32, 32) and (32, 32, -32).
            // The new vertices are indented.
            // So the MAX X and MAX Y should be slightly less than 32?
            // No, the Right face is still at X=32 (except near the bevel).
            // The Back face is still at Y=32.
            // The Bevel cuts the corner.
            // So the bounding box should still touch 32 on X and Y (at the non-beveled corners).
            // Wait, this is a CUBE.
            // The edge at (32,32) is the ONLY edge at X=32, Y=32?
            // Yes, for that Z range.
            // So X=32 is only reached at that edge? 
            // No.
            // Top-Right edge is at X=32, Z=32. Y goes from -32 to 32.
            // So at Y=-32, X is still 32.
            // So the bounding box MAX should still be 32.
            CHECK(brush.bounds() == vm::bbox3d{{-32, -32, -32}, {32, 32, 32}});
        }

        SECTION("Bevel distance too large fails or clips entire brush?")
        {
             // If bevel distance is large (e.g. 100), the plane moves past the origin.
             // It might clip the whole brush away or result in valid smaller brush.
             // 60, 60, 60.
             // Plane normal (1,1,0). Normalized ~ (0.7, 0.7, 0).
             // Edge at (32,32).
             // Plane point: (32,32) - n * 100.
             // Point (-38, -38).
             // The plane faces (1,1,0).
             // So it keeps the side towards (32,32)? No, normal points OUT of original faces.
             // `nBevel = n1 + n2`. n1=(1,0,0), n2=(0,1,0). nBevel=(1,1,0).
             // We move point IN: P - n * Dist.
             // We keep the side BEHIND the plane?
             // `BrushFace(plane)` constructor creates a face.
             // `clip` keeps the volume inside the brush and inside the half-space defined by the face?
             // `Brush::clip` keeps the part *behind* the plane? Or *in front*?
             // Usually `clip(plane)` keeps the "inside".
             // `BrushFace` normal points OUT.
             // So we keep the side OPPOSITE to the normal.
             // Our Bevel Plane normal is `nBevel` (pointing OUT).
             // So we keep the side OPPOSITE to `nBevel`.
             // If we move the plane deep inside (past origin), we keep the "back" of the brush (the -X, -Y side).
             // So the brush should shrink but remain valid.
             
            auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
            auto brush = builder.createCuboid(vm::bbox3d{{-32, -32, -32}, {32, 32, 32}}, "material")
                       | kdl::value();
                       
            vm::segment3d targetEdge{ {32, 32, -32}, {32, 32, 32} };
            double hugeDist = 50.0; // Cube width 64. Diagonal from center to corner is sqrt(32^2+32^2) = 45.
            // 50 should cross the origin.
            
            auto result = brush.bevelEdge(worldBounds, targetEdge, hugeDist, false);
            // Result should be valid, just weird shape.
            REQUIRE(result.is_success());
            CHECK(brush.vertexCount() > 0);
        }
    }
}
