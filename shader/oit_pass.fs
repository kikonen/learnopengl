#version 460 core

// Forward-lit transparent pass. Reuses the deferred PBR + IBL lighting (calculateLightPbr)
// so transparent surfaces respond to the environment and day/night just like opaque ones,
// instead of emitting full albedo (which made them "glow" in the dark).
#define PASS_FORWARD

#include "include/ssbo_materials.glsl"

#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"
#include "include/uniform_shadow.glsl"
#include "include/uniform_debug.glsl"
#include "include/uniform_lights.glsl"

#include "include/water_caustics.glsl"

// NOTE KI depth is *not* updated in OIT pass
// => testing against solid depth
// NOTE KI "early_fragment_tests" cannot be used at same same with alpha
// => disables "discard" logic
//layout(early_fragment_tests) in;

in VS_OUT {
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
  flat uint flags;
} fs_in;

LAYOUT_OIT_OUT;

layout(binding = UNIT_IRRADIANCE_MAP) uniform samplerCube u_irradianceMap;
layout(binding = UNIT_PREFILTER_MAP) uniform samplerCube u_prefilterMap;
layout(binding = UNIT_BRDF_LUT) uniform sampler2D u_brdfLut;

layout(binding = UNIT_SHADOW_MAP_FIRST) uniform sampler2DShadow u_shadowMap[MAX_SHADOW_MAP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include "include/pbr.glsl"
#include "include/fn_calculate_dir_light.glsl"
#include "include/fn_calculate_point_light.glsl"
#include "include/fn_calculate_spot_light.glsl"
#include "include/fn_calculate_light.glsl"
#include "include/fn_calculate_shadow_index.glsl"

#include "include/fn_oit_util.glsl"

void main()
{
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;

  #include "include/var_tex_material_alpha.glsl"
  OIT_DISCARD(alpha);

  #include "include/var_tex_material.glsl"

  const vec3 viewPos = fs_in.viewPos;
  const vec3 worldPos = (u_invViewMatrix * vec4(viewPos, 1)).xyz;
  const vec3 viewNormal = normalize(fs_in.normal);

  // Modulate ALBEDO with caustics before lighting (same as g_tex/g_terrain), so the
  // caustic gets lit and fades at night instead of glowing on the dimmed surface.
  applyWaterCaustic(material.diffuse.rgb, worldPos);

  const uint shadowIndex = calculateShadowIndex(viewPos);

  // full PBR + IBL ambient (same as deferred); returns vec4(litColor, material alpha)
  vec4 color = calculateLightPbr(viewNormal, viewPos, worldPos, shadowIndex);

  float weight = clamp(pow(min(1.0, alpha * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

  o_accum = vec4(color.rgb * alpha, alpha) * weight;
  o_reveal = alpha;

  o_fragEmission = material.emission;
}
