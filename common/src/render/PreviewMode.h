/*
 Copyright (C) 2024 TrenchBroom Contributors

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

namespace tb::render
{

/**
 * Preview rendering modes for enhanced visualization
 */
enum class PreviewMode
{
  /**
   * Standard rendering with materials
   */
  Normal,
  
  /**
   * Lighting preview simulating in-game lighting
   */
  LightingPreview,
  
  /**
   * Ambient occlusion preview for soft shadows
   */
  AmbientOcclusion,
  
  /**
   * Shadow preview showing shadow casting
   */
  ShadowPreview,
  
  /**
   * Flat shading with no materials
   */
  FlatShaded,
  
  /**
   * Wireframe rendering
   */
  Wireframe,
  
  /**
   * X-ray mode showing occluded geometry
   */
  Xray
};

/**
 * Settings for enhanced rendering preview
 */
struct PreviewSettings
{
  PreviewMode mode = PreviewMode::Normal;
  
  // Lighting preview settings
  float lightIntensity = 1.0f;
  float ambientIntensity = 0.3f;
  bool showLightSources = true;
  
  // Ambient occlusion settings
  float aoRadius = 32.0f;
  float aoIntensity = 0.5f;
  
  // Shadow settings
  float shadowBias = 0.001f;
  float shadowSoftness = 0.5f;
  
  // X-ray settings
  float xrayOpacity = 0.3f;
};

} // namespace tb::render
