/*
 Copyright (C) 2023 Kristian Duske

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

#include "ui/DrawShapeToolExtensions.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QStackedWidget>
#include <QToolButton>

#include "mdl/BrushBuilder.h"
#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h"
#include "mdl/Map.h"
#include "mdl/WorldNode.h"
#include "ui/BitmapButton.h"
#include "ui/MapDocument.h"
#include "ui/ViewConstants.h"

namespace tb::ui
{

DrawShapeToolCuboidExtension::DrawShapeToolCuboidExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolCuboidExtension::name() const
{
  static const auto name = std::string{"Cuboid"};
  return name;
}

const std::filesystem::path& DrawShapeToolCuboidExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_Cuboid.svg"};
  return path;
}

DrawShapeToolExtensionPage* DrawShapeToolCuboidExtension::createToolPage(
  ShapeParameters&, QWidget* parent)
{
  return new DrawShapeToolExtensionPage{parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolCuboidExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters&) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};

  return builder.createCuboid(bounds, map.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolAxisAlignedShapeExtensionPage::DrawShapeToolAxisAlignedShapeExtensionPage(
  ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolExtensionPage{parent}
  , m_parameters{parameters}
{
  auto* axisLabel = new QLabel{tr("Axis: ")};
  auto* axisComboBox = new QComboBox{};
  axisComboBox->addItems({tr("X"), tr("Y"), tr("Z")});

  connect(
    axisComboBox,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    [&](const auto index) { m_parameters.setAxis(vm::axis::type(index)); });

  addWidget(axisLabel);
  addWidget(axisComboBox);

  const auto updateWidgets = [=, this]() {
    axisComboBox->setCurrentIndex(int(m_parameters.axis()));
  };
  updateWidgets();

  m_notifierConnection +=
    m_parameters.parametersDidChangeNotifier.connect(std::move(updateWidgets));
}

DrawShapeToolCircularShapeExtensionPage::DrawShapeToolCircularShapeExtensionPage(
  ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolAxisAlignedShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  auto* numSidesLabel = new QLabel{tr("Number of Sides: ")};
  auto* numSidesBox = new QSpinBox{};
  numSidesBox->setRange(3, 96);

  auto* precisionBox = new QComboBox{};
  precisionBox->addItems({"12", "24", "48", "96"});

  auto* numSidesWidget = new QStackedWidget{};
  numSidesWidget->addWidget(numSidesBox);
  numSidesWidget->addWidget(precisionBox);

  auto* edgeAlignedCircleButton =
    createBitmapToggleButton("CircleEdgeAligned.svg", tr("Align edge to bounding box"));
  edgeAlignedCircleButton->setIconSize({24, 24});
  edgeAlignedCircleButton->setObjectName("toolButton_withBorder");

  auto* vertexAlignedCircleButton = createBitmapToggleButton(
    "CircleVertexAligned.svg", tr("Align vertices to bounding box"));
  vertexAlignedCircleButton->setIconSize({24, 24});
  vertexAlignedCircleButton->setObjectName("toolButton_withBorder");

  auto* scalableCircleButton =
    createBitmapToggleButton("CircleScalable.svg", tr("Scalable circle shape"));
  scalableCircleButton->setIconSize({24, 24});
  scalableCircleButton->setObjectName("toolButton_withBorder");

  auto* radiusModeButtonGroup = new QButtonGroup{};
  radiusModeButtonGroup->addButton(edgeAlignedCircleButton);
  radiusModeButtonGroup->addButton(vertexAlignedCircleButton);
  radiusModeButtonGroup->addButton(scalableCircleButton);

  connect(
    numSidesBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto numSides) {
      m_parameters.setCircleShape(std::visit(
        kdl::overload(
          [&](const mdl::EdgeAlignedCircle&) -> mdl::CircleShape {
            return mdl::EdgeAlignedCircle{size_t(numSides)};
          },
          [&](const mdl::VertexAlignedCircle&) -> mdl::CircleShape {
            return mdl::VertexAlignedCircle{size_t(numSides)};
          },
          [&](const mdl::ScalableCircle& circleShape) -> mdl::CircleShape {
            return circleShape;
          }),
        m_parameters.circleShape()));
    });
  connect(
    precisionBox,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    [&](const auto precision) {
      m_parameters.setCircleShape(std::visit(
        kdl::overload(
          [&](const mdl::ScalableCircle&) -> mdl::CircleShape {
            return mdl::ScalableCircle{size_t(precision)};
          },
          [](const auto& circleShape) -> mdl::CircleShape { return circleShape; }),
        m_parameters.circleShape()));
    });
  connect(edgeAlignedCircleButton, &QToolButton::clicked, this, [=, this]() {
    m_parameters.setCircleShape(
      mdl::convertCircleShape<mdl::EdgeAlignedCircle>(m_parameters.circleShape()));
  });
  connect(vertexAlignedCircleButton, &QToolButton::clicked, this, [=, this]() {
    m_parameters.setCircleShape(
      mdl::convertCircleShape<mdl::VertexAlignedCircle>(m_parameters.circleShape()));
  });
  connect(scalableCircleButton, &QToolButton::clicked, this, [=, this]() {
    m_parameters.setCircleShape(
      mdl::convertCircleShape<mdl::ScalableCircle>(m_parameters.circleShape()));
  });

  addWidget(numSidesLabel);
  addWidget(numSidesWidget);
  addWidget(edgeAlignedCircleButton);
  addWidget(vertexAlignedCircleButton);
  addWidget(scalableCircleButton);

  const auto updateWidgets = [=, this]() {
    std::visit(
      kdl::overload(
        [&](const mdl::EdgeAlignedCircle& circleShape) {
          numSidesBox->setValue(int(circleShape.numSides));
          numSidesWidget->setCurrentWidget(numSidesBox);
        },
        [&](const mdl::VertexAlignedCircle& circleShape) {
          numSidesBox->setValue(int(circleShape.numSides));
          numSidesWidget->setCurrentWidget(numSidesBox);
        },
        [&](const mdl::ScalableCircle& circleShape) {
          precisionBox->setCurrentIndex(int(circleShape.precision));
          numSidesWidget->setCurrentWidget(precisionBox);
        }),
      m_parameters.circleShape());

    edgeAlignedCircleButton->setChecked(
      std::holds_alternative<mdl::EdgeAlignedCircle>(m_parameters.circleShape()));
    vertexAlignedCircleButton->setChecked(
      std::holds_alternative<mdl::VertexAlignedCircle>(m_parameters.circleShape()));
    scalableCircleButton->setChecked(
      std::holds_alternative<mdl::ScalableCircle>(m_parameters.circleShape()));
  };
  updateWidgets();

  m_notifierConnection +=
    m_parameters.parametersDidChangeNotifier.connect(std::move(updateWidgets));
}

DrawShapeToolCylinderShapeExtensionPage::DrawShapeToolCylinderShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolCircularShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  auto* hollowCheckBox = new QCheckBox{tr("Hollow")};

  auto* thicknessLabel = new QLabel{tr("Thickness: ")};
  auto* thicknessBox = new QDoubleSpinBox{};
  thicknessBox->setEnabled(parameters.hollow());
  thicknessBox->setRange(1, 128);

  connect(hollowCheckBox, &QCheckBox::toggled, this, [&](const auto hollow) {
    m_parameters.setHollow(hollow);
  });
  connect(
    thicknessBox,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    [&](const auto thickness) { m_parameters.setThickness(thickness); });

  addWidget(hollowCheckBox);
  addWidget(thicknessLabel);
  addWidget(thicknessBox);
  addApplyButton(document);

  const auto updateWidgets = [=, this]() {
    hollowCheckBox->setChecked(m_parameters.hollow());
    thicknessBox->setEnabled(m_parameters.hollow());
    thicknessBox->setValue(m_parameters.thickness());
  };
  updateWidgets();

  m_notifierConnection +=
    m_parameters.parametersDidChangeNotifier.connect(std::move(updateWidgets));
}

DrawShapeToolCylinderExtension::DrawShapeToolCylinderExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolCylinderExtension::name() const
{
  static const auto name = std::string{"Cylinder"};
  return name;
}

const std::filesystem::path& DrawShapeToolCylinderExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_Cylinder.svg"};
  return path;
}

DrawShapeToolExtensionPage* DrawShapeToolCylinderExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolCylinderShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolCylinderExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};
  return parameters.hollow()
           ? builder.createHollowCylinder(
               bounds,
               parameters.thickness(),
               parameters.circleShape(),
               parameters.axis(),
               map.currentMaterialName())
           : builder
               .createCylinder(
                 bounds,
                 parameters.circleShape(),
                 parameters.axis(),
                 map.currentMaterialName())
               .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolConeShapeExtensionPage::DrawShapeToolConeShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolCircularShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  addApplyButton(document);
}

DrawShapeToolConeExtension::DrawShapeToolConeExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolConeExtension::name() const
{
  static const auto name = std::string{"Cone"};
  return name;
}

const std::filesystem::path& DrawShapeToolConeExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_Cone.svg"};
  return path;
}

DrawShapeToolExtensionPage* DrawShapeToolConeExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolConeShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolConeExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};
  return builder
    .createCone(
      bounds, parameters.circleShape(), parameters.axis(), map.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolIcoSphereShapeExtensionPage::DrawShapeToolIcoSphereShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolExtensionPage{parent}
  , m_parameters{parameters}
{
  auto* accuracyLabel = new QLabel{tr("Accuracy: ")};
  auto* accuracyBox = new QSpinBox{};
  accuracyBox->setRange(0, 4);

  connect(
    accuracyBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto accuracy) { m_parameters.setAccuracy(size_t(accuracy)); });

  addWidget(accuracyLabel);
  addWidget(accuracyBox);
  addApplyButton(document);

  const auto updateWidgets = [=, this]() {
    accuracyBox->setValue(int(m_parameters.accuracy()));
  };
  updateWidgets();

  m_notifierConnection +=
    m_parameters.parametersDidChangeNotifier.connect(std::move(updateWidgets));
}

DrawShapeToolIcoSphereExtension::DrawShapeToolIcoSphereExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolIcoSphereExtension::name() const
{
  static const auto name = std::string{"Spheroid (Icosahedron)"};
  return name;
}

const std::filesystem::path& DrawShapeToolIcoSphereExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_IcoSphere.svg"};
  return path;
}

DrawShapeToolExtensionPage* DrawShapeToolIcoSphereExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolIcoSphereShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolIcoSphereExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};

  return builder.createIcoSphere(bounds, parameters.accuracy(), map.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

DrawShapeToolUVSphereShapeExtensionPage::DrawShapeToolUVSphereShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolCircularShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  auto* numRingsLabel = new QLabel{tr("Number of Rings: ")};
  auto* numRingsBox = new QSpinBox{};
  numRingsBox->setRange(1, 256);

  auto* numRingsLayout = new QHBoxLayout{};
  numRingsLayout->setContentsMargins(QMargins{});
  numRingsLayout->setSpacing(LayoutConstants::MediumHMargin);
  numRingsLayout->addWidget(numRingsLabel);
  numRingsLayout->addWidget(numRingsBox);

  auto* numRingsWidget = new QWidget{};
  numRingsWidget->setLayout(numRingsLayout);

  connect(
    numRingsBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto numRings) { m_parameters.setNumRings(size_t(numRings)); });

  addWidget(numRingsWidget);
  addApplyButton(document);

  const auto updateWidgets = [=, this]() {
    numRingsWidget->setVisible(
      !std::holds_alternative<mdl::ScalableCircle>(m_parameters.circleShape()));
    numRingsBox->setValue(int(m_parameters.numRings()));
  };
  updateWidgets();

  m_notifierConnection +=
    m_parameters.parametersDidChangeNotifier.connect(std::move(updateWidgets));
}

DrawShapeToolUVSphereExtension::DrawShapeToolUVSphereExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolUVSphereExtension::name() const
{
  static const auto name = std::string{"Spheroid (UV)"};
  return name;
}

const std::filesystem::path& DrawShapeToolUVSphereExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_UVSphere.svg"};
  return path;
}

DrawShapeToolExtensionPage* DrawShapeToolUVSphereExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolUVSphereShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolUVSphereExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};
  return builder
    .createUVSphere(
      bounds,
      parameters.circleShape(),
      parameters.numRings(),
      parameters.axis(),
      map.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

std::vector<std::unique_ptr<DrawShapeToolExtension>> createDrawShapeToolExtensions(
  MapDocument& document)
{
  auto result = std::vector<std::unique_ptr<DrawShapeToolExtension>>{};
  result.push_back(std::make_unique<DrawShapeToolCuboidExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolCylinderExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolConeExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolUVSphereExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolIcoSphereExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolWedgeExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolStaircaseExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolArchExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolPipeExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolTorusExtension>(document));
  result.push_back(std::make_unique<DrawShapeToolTerrainGridExtension>(document));
  return result;
}

// --- Wedge Extension ---

DrawShapeToolWedgeShapeExtensionPage::DrawShapeToolWedgeShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolAxisAlignedShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  addApplyButton(document);
}

DrawShapeToolWedgeExtension::DrawShapeToolWedgeExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolWedgeExtension::name() const
{
  static const auto name = std::string{"Wedge"};
  return name;
}

const std::filesystem::path& DrawShapeToolWedgeExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_Wedge.svg"};
  return path;
}

DrawShapeToolExtensionPage* DrawShapeToolWedgeExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolWedgeShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolWedgeExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};

  return builder.createWedge(bounds, parameters.axis(), map.currentMaterialName())
    .transform([](auto brush) { return std::vector{std::move(brush)}; });
}

// --- Staircase Extension ---

DrawShapeToolStaircaseShapeExtensionPage::DrawShapeToolStaircaseShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolAxisAlignedShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  auto* numStepsLabel = new QLabel{tr("Number of Steps: ")};
  auto* numStepsBox = new QSpinBox{};
  numStepsBox->setRange(1, 128);

  connect(
    numStepsBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto numSteps) { m_parameters.setNumSteps(size_t(numSteps)); });

  addWidget(numStepsLabel);
  addWidget(numStepsBox);
  addApplyButton(document);

  const auto updateWidgets = [=, this]() {
    numStepsBox->setValue(int(m_parameters.numSteps()));
  };
  updateWidgets();

  m_notifierConnection +=
    m_parameters.parametersDidChangeNotifier.connect(std::move(updateWidgets));
}

DrawShapeToolStaircaseExtension::DrawShapeToolStaircaseExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolStaircaseExtension::name() const
{
  static const auto name = std::string{"Staircase"};
  return name;
}

const std::filesystem::path& DrawShapeToolStaircaseExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_Staircase.svg"};
  return path;
}

DrawShapeToolExtensionPage* DrawShapeToolStaircaseExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolStaircaseShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolStaircaseExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};
  return builder.createStaircase(
    bounds, parameters.numSteps(), parameters.axis(), map.currentMaterialName());
}

// --- Arch Extension ---

DrawShapeToolArchShapeExtensionPage::DrawShapeToolArchShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolCircularShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  auto* numSlicesLabel = new QLabel{tr("Slices: ")};
  auto* numSlicesBox = new QSpinBox{};
  numSlicesBox->setRange(1, 64);

  auto* arcLabel = new QLabel{tr("Arc (\u00B0): ")};
  auto* arcBox = new QDoubleSpinBox{};
  arcBox->setRange(1.0, 360.0);
  arcBox->setSingleStep(5.0);

  auto* thicknessLabel = new QLabel{tr("Thickness: ")};
  auto* thicknessBox = new QDoubleSpinBox{};
  thicknessBox->setRange(1.0, 1024.0);

  connect(
    numSlicesBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto n) { m_parameters.setNumSlices(size_t(n)); });
  connect(
    arcBox,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    [&](const auto a) { m_parameters.setArcDegrees(a); });
  connect(
    thicknessBox,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    [&](const auto t) { m_parameters.setThickness(t); });

  addWidget(numSlicesLabel);
  addWidget(numSlicesBox);
  addWidget(arcLabel);
  addWidget(arcBox);
  addWidget(thicknessLabel);
  addWidget(thicknessBox);
  addApplyButton(document);

  const auto updateWidgets = [=, this]() {
    numSlicesBox->setValue(int(m_parameters.numSlices()));
    arcBox->setValue(m_parameters.arcDegrees());
    thicknessBox->setValue(m_parameters.thickness());
  };
  updateWidgets();

  m_notifierConnection +=
    m_parameters.parametersDidChangeNotifier.connect(std::move(updateWidgets));
}

DrawShapeToolArchExtension::DrawShapeToolArchExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolArchExtension::name() const
{
  static const auto name = std::string{"Arch"};
  return name;
}

const std::filesystem::path& DrawShapeToolArchExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_Arch.svg"};
  return path;
}

DrawShapeToolExtensionPage* DrawShapeToolArchExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolArchShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolArchExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};
  return builder.createArch(
    bounds,
    parameters.numSlices(),
    parameters.arcDegrees(),
    parameters.thickness(),
    parameters.circleShape(),
    parameters.axis(),
    map.currentMaterialName());
}

// --- Pipe Extension ---

DrawShapeToolPipeShapeExtensionPage::DrawShapeToolPipeShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolCircularShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  auto* thicknessLabel = new QLabel{tr("Thickness: ")};
  auto* thicknessBox = new QDoubleSpinBox{};
  thicknessBox->setRange(1, 128);

  connect(
    thicknessBox,
    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this,
    [&](const auto thickness) { m_parameters.setThickness(thickness); });

  addWidget(thicknessLabel);
  addWidget(thicknessBox);
  addApplyButton(document);

  const auto updateWidgets = [=, this]() {
    thicknessBox->setValue(m_parameters.thickness());
  };
  updateWidgets();

  m_notifierConnection +=
    m_parameters.parametersDidChangeNotifier.connect(std::move(updateWidgets));
}

DrawShapeToolPipeExtension::DrawShapeToolPipeExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolPipeExtension::name() const
{
  static const auto name = std::string{"Pipe"};
  return name;
}

const std::filesystem::path& DrawShapeToolPipeExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_Pipe.svg"};
  return path;
}

DrawShapeToolExtensionPage* DrawShapeToolPipeExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolPipeShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolPipeExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};
  return builder.createPipe(
    bounds,
    parameters.thickness(),
    parameters.circleShape(),
    parameters.axis(),
    map.currentMaterialName());
}

// --- Torus Extension ---

DrawShapeToolTorusShapeExtensionPage::DrawShapeToolTorusShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolAxisAlignedShapeExtensionPage{parameters, parent}
  , m_parameters{parameters}
{
  auto* ringSegLabel = new QLabel{tr("Ring Segments: ")};
  auto* ringSegBox = new QSpinBox{};
  ringSegBox->setRange(3, 64);

  auto* tubeSegLabel = new QLabel{tr("Tube Segments: ")};
  auto* tubeSegBox = new QSpinBox{};
  tubeSegBox->setRange(3, 32);

  connect(
    ringSegBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto n) { m_parameters.setNumRingSegments(size_t(n)); });
  connect(
    tubeSegBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto n) { m_parameters.setNumTubeSegments(size_t(n)); });

  addWidget(ringSegLabel);
  addWidget(ringSegBox);
  addWidget(tubeSegLabel);
  addWidget(tubeSegBox);
  addApplyButton(document);

  const auto updateWidgets = [=, this]() {
    ringSegBox->setValue(int(m_parameters.numRingSegments()));
    tubeSegBox->setValue(int(m_parameters.numTubeSegments()));
  };
  updateWidgets();

  m_notifierConnection +=
    m_parameters.parametersDidChangeNotifier.connect(std::move(updateWidgets));
}

DrawShapeToolTorusExtension::DrawShapeToolTorusExtension(MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolTorusExtension::name() const
{
  static const auto name = std::string{"Torus"};
  return name;
}

const std::filesystem::path& DrawShapeToolTorusExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_Torus.svg"};
  return path;
}

DrawShapeToolExtensionPage* DrawShapeToolTorusExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolTorusShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolTorusExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};
  return builder.createTorus(
    bounds,
    parameters.numRingSegments(),
    parameters.numTubeSegments(),
    parameters.axis(),
    map.currentMaterialName());
}

// --- Terrain Grid Extension ---

DrawShapeToolTerrainGridShapeExtensionPage::DrawShapeToolTerrainGridShapeExtensionPage(
  MapDocument& document, ShapeParameters& parameters, QWidget* parent)
  : DrawShapeToolExtensionPage{parent}
  , m_parameters{parameters}
{
  auto* rowsLabel = new QLabel{tr("Rows: ")};
  auto* rowsBox = new QSpinBox{};
  rowsBox->setRange(1, 64);

  auto* colsLabel = new QLabel{tr("Columns: ")};
  auto* colsBox = new QSpinBox{};
  colsBox->setRange(1, 64);

  connect(
    rowsBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto n) { m_parameters.setGridRows(size_t(n)); });
  connect(
    colsBox,
    QOverload<int>::of(&QSpinBox::valueChanged),
    this,
    [&](const auto n) { m_parameters.setGridCols(size_t(n)); });

  addWidget(rowsLabel);
  addWidget(rowsBox);
  addWidget(colsLabel);
  addWidget(colsBox);
  addApplyButton(document);

  const auto updateWidgets = [=, this]() {
    rowsBox->setValue(int(m_parameters.gridRows()));
    colsBox->setValue(int(m_parameters.gridCols()));
  };
  updateWidgets();

  m_notifierConnection +=
    m_parameters.parametersDidChangeNotifier.connect(std::move(updateWidgets));
}

DrawShapeToolTerrainGridExtension::DrawShapeToolTerrainGridExtension(
  MapDocument& document)
  : DrawShapeToolExtension{document}
{
}

const std::string& DrawShapeToolTerrainGridExtension::name() const
{
  static const auto name = std::string{"Terrain Grid"};
  return name;
}

const std::filesystem::path& DrawShapeToolTerrainGridExtension::iconPath() const
{
  static const auto path = std::filesystem::path{"ShapeTool_TerrainGrid.svg"};
  return path;
}

DrawShapeToolExtensionPage* DrawShapeToolTerrainGridExtension::createToolPage(
  ShapeParameters& parameters, QWidget* parent)
{
  return new DrawShapeToolTerrainGridShapeExtensionPage{m_document, parameters, parent};
}

Result<std::vector<mdl::Brush>> DrawShapeToolTerrainGridExtension::createBrushes(
  const vm::bbox3d& bounds, const ShapeParameters& parameters) const
{
  auto& map = m_document.map();

  const auto builder = mdl::BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};
  return builder.createTerrainGrid(
    bounds, parameters.gridRows(), parameters.gridCols(), map.currentMaterialName());
}

} // namespace tb::ui
