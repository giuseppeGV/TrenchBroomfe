#include "Matchers.h"
#include "TestUtils.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Picking.h"
#include "mdl/Map_Selection.h"
#include "mdl/PickResult.h"
#include "mdl/WorldNode.h"
#include "ui/ExtrudeTool.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"

#include "kd/result.h"
#include "vm/vec.h"
#include "vm/bbox.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
using namespace Catch::Matchers;

TEST_CASE("ExtrudeTool Inset")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto tool = ExtrudeTool{document};

  // Create a 32x32x32 cube centered at origin
  constexpr auto brushBounds = vm::bbox3d{16.0};
  auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
  auto* brushNode =
    new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};

  addNodes(map, {{map.editorContext().currentLayer(), {brushNode}}});
  selectNodes(map, {brushNode});

  // Pick the Top Face (Normal 0,0,1)
  const auto pickRay = vm::ray3d{{0, 0, 32}, {0, 0, -1}};
  auto pickResult = mdl::PickResult::byDistance();
  pick(map, pickRay, pickResult);
  
  const auto hit = tool.pick3D(pickRay, pickResult);
  REQUIRE(hit.isMatch());
  pickResult.addHit(hit);
  tool.updateProposedDragHandles(pickResult);
  
  REQUIRE(!tool.proposedDragHandles().empty());
  
  // Setup Drag State
  // Center of top face is (0,0,16).
  // Hit point (initial click) at (10, 0, 16) -> distance 10 from center.
  vm::vec3d initialClick = {10, 0, 16};
  
  // We simulate a drag STATE initialization similar to createInsetDragTracker
  auto dragState = ExtrudeDragState{
      tool.proposedDragHandles(),
      ExtrudeTool::getDragFaces(tool.proposedDragHandles()),
      false, 
      {0,0,0}, 
      initialClick
  };

  tool.beginInset();

  SECTION("Inset Inwards (Scale Down)")
  {
      // Move 5 units inwards towards center.
      // Current Point = (5, 0, 16).
      // Delta = (-5, 0, 0).
      vm::vec3d delta = {-5, 0, 0};
      
      REQUIRE(tool.inset(delta, dragState));
      tool.commit(dragState);
      
      // Verification
      // Initial radius = 10. New radius = 5. Scale = 0.5.
      // Original face bounds: X from -16 to 16. Y from -16 to 16.
      // New face bounds should be scaled by 0.5 -> -8 to 8.
      
      const auto& brush = brushNode->brush();
      const auto newBounds = brush.bounds();
      // Only the TOP face was scaled. The bottom face is unchanged (-16..16).
      // So the bounding box of the whole brush is STILL [-16..16] in X/Y because the bottom is wide.
      // We need to check the geometry of the top face specifically.
      
      auto topFaceIndex = brush.findFace(vm::vec3d{0,0,1});
      REQUIRE(topFaceIndex.has_value());
      const auto& topFace = brush.face(*topFaceIndex);
      
      // Check vertices of the polygon
      const auto poly = topFace.polygon();
      const auto bbox = vm::bbox3d::merge_all(poly.vertices().begin(), poly.vertices().end());
      
      // Approx check
      CHECK(bbox.min.x() == Catch::Approx(-8.0).margin(0.1));
      CHECK(bbox.max.x() == Catch::Approx(8.0).margin(0.1));
      CHECK(bbox.min.y() == Catch::Approx(-8.0).margin(0.1));
      CHECK(bbox.max.y() == Catch::Approx(8.0).margin(0.1));
  }

  SECTION("Inset Outwards (Scale Up)")
  {
      // Move 5 units outwards away from center.
      // Current Point = (15, 0, 16).
      // Delta = (5, 0, 0).
      vm::vec3d delta = {5, 0, 0};
      
      REQUIRE(tool.inset(delta, dragState));
      tool.commit(dragState);
      
      // Radius 10 -> 15. Scale = 1.5.
      // 16 * 1.5 = 24.
      
      const auto& brush = brushNode->brush();
      auto topFaceIndex = brush.findFace(vm::vec3d{0,0,1});
      REQUIRE(topFaceIndex.has_value());
      const auto& topFace = brush.face(*topFaceIndex);
      
      const auto poly = topFace.polygon();
      const auto bbox = vm::bbox3d::merge_all(poly.vertices().begin(), poly.vertices().end());
      
      CHECK(bbox.min.x() == Catch::Approx(-24.0).margin(0.1));
      CHECK(bbox.max.x() == Catch::Approx(24.0).margin(0.1));
  }
}

} // namespace tb::ui
