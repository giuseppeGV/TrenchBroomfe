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

#pragma once

#include "Result.h"
#include "mdl/BrushBuilder.h"
#include "ui/DrawShapeToolExtension.h"

#include <vector>

class QWidget;

namespace tb::ui
{
class MapDocument;

class DrawShapeToolCuboidExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolCuboidExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(
    ShapeParameters& parameters, QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const ShapeParameters& parameters) const override;
};

class DrawShapeToolAxisAlignedShapeExtensionPage : public DrawShapeToolExtensionPage
{
public:
  explicit DrawShapeToolAxisAlignedShapeExtensionPage(
    ShapeParameters& parameters, QWidget* parent = nullptr);

private:
  ShapeParameters& m_parameters;
};

class DrawShapeToolCircularShapeExtensionPage
  : public DrawShapeToolAxisAlignedShapeExtensionPage
{
public:
  explicit DrawShapeToolCircularShapeExtensionPage(
    ShapeParameters& parameters, QWidget* parent = nullptr);

private:
  ShapeParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolCylinderShapeExtensionPage
  : public DrawShapeToolCircularShapeExtensionPage
{
public:
  explicit DrawShapeToolCylinderShapeExtensionPage(
    MapDocument& document, ShapeParameters& parameters, QWidget* parent = nullptr);

private:
  ShapeParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolCylinderExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolCylinderExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(
    ShapeParameters& parameters, QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const ShapeParameters& parameters) const override;
};

class DrawShapeToolConeShapeExtensionPage : public DrawShapeToolCircularShapeExtensionPage
{
public:
  explicit DrawShapeToolConeShapeExtensionPage(
    MapDocument& document, ShapeParameters& parameters, QWidget* parent = nullptr);

private:
  ShapeParameters& m_parameters;
  Q_OBJECT
};

class DrawShapeToolConeExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolConeExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(
    ShapeParameters& parameters, QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const ShapeParameters& parameters) const override;
};

class DrawShapeToolIcoSphereShapeExtensionPage : public DrawShapeToolExtensionPage
{
public:
  explicit DrawShapeToolIcoSphereShapeExtensionPage(
    MapDocument& document, ShapeParameters& parameters, QWidget* parent = nullptr);

private:
  ShapeParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolIcoSphereExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolIcoSphereExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(
    ShapeParameters& parameters, QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const ShapeParameters& parameters) const override;
};

class DrawShapeToolUVSphereShapeExtensionPage
  : public DrawShapeToolCircularShapeExtensionPage
{
public:
  explicit DrawShapeToolUVSphereShapeExtensionPage(
    MapDocument& document, ShapeParameters& parameters, QWidget* parent = nullptr);

private:
  ShapeParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolUVSphereExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolUVSphereExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(
    ShapeParameters& parameters, QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const ShapeParameters& parameters) const override;
};

// --- New shape extensions ---

class DrawShapeToolWedgeShapeExtensionPage
  : public DrawShapeToolAxisAlignedShapeExtensionPage
{
public:
  explicit DrawShapeToolWedgeShapeExtensionPage(
    MapDocument& document, ShapeParameters& parameters, QWidget* parent = nullptr);

private:
  ShapeParameters& m_parameters;
  Q_OBJECT
};

class DrawShapeToolWedgeExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolWedgeExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(
    ShapeParameters& parameters, QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const ShapeParameters& parameters) const override;
};

class DrawShapeToolStaircaseShapeExtensionPage
  : public DrawShapeToolAxisAlignedShapeExtensionPage
{
public:
  explicit DrawShapeToolStaircaseShapeExtensionPage(
    MapDocument& document, ShapeParameters& parameters, QWidget* parent = nullptr);

private:
  ShapeParameters& m_parameters;
  Q_OBJECT
};

class DrawShapeToolStaircaseExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolStaircaseExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(
    ShapeParameters& parameters, QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const ShapeParameters& parameters) const override;
};

class DrawShapeToolArchShapeExtensionPage : public DrawShapeToolCircularShapeExtensionPage
{
public:
  explicit DrawShapeToolArchShapeExtensionPage(
    MapDocument& document, ShapeParameters& parameters, QWidget* parent = nullptr);

private:
  ShapeParameters& m_parameters;
  Q_OBJECT
};

class DrawShapeToolArchExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolArchExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(
    ShapeParameters& parameters, QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const ShapeParameters& parameters) const override;
};

class DrawShapeToolPipeShapeExtensionPage : public DrawShapeToolCircularShapeExtensionPage
{
public:
  explicit DrawShapeToolPipeShapeExtensionPage(
    MapDocument& document, ShapeParameters& parameters, QWidget* parent = nullptr);

private:
  ShapeParameters& m_parameters;
  Q_OBJECT
};

class DrawShapeToolPipeExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolPipeExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(
    ShapeParameters& parameters, QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const ShapeParameters& parameters) const override;
};

class DrawShapeToolTorusShapeExtensionPage
  : public DrawShapeToolAxisAlignedShapeExtensionPage
{
public:
  explicit DrawShapeToolTorusShapeExtensionPage(
    MapDocument& document, ShapeParameters& parameters, QWidget* parent = nullptr);

private:
  ShapeParameters& m_parameters;
  Q_OBJECT
};

class DrawShapeToolTorusExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolTorusExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(
    ShapeParameters& parameters, QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const ShapeParameters& parameters) const override;
};

class DrawShapeToolTerrainGridShapeExtensionPage : public DrawShapeToolExtensionPage
{
public:
  explicit DrawShapeToolTerrainGridShapeExtensionPage(
    MapDocument& document, ShapeParameters& parameters, QWidget* parent = nullptr);

private:
  ShapeParameters& m_parameters;
  Q_OBJECT
};

class DrawShapeToolTerrainGridExtension : public DrawShapeToolExtension
{
public:
  explicit DrawShapeToolTerrainGridExtension(MapDocument& document);

  const std::string& name() const override;
  const std::filesystem::path& iconPath() const override;
  DrawShapeToolExtensionPage* createToolPage(
    ShapeParameters& parameters, QWidget* parent) override;
  Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds, const ShapeParameters& parameters) const override;
};

std::vector<std::unique_ptr<DrawShapeToolExtension>> createDrawShapeToolExtensions(
  MapDocument& document);

} // namespace tb::ui
