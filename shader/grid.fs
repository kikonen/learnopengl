#version 460 core

#include include/ssbo_materials.glsl

#include include/uniform_camera.glsl
#include include/uniform_data.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  vec3 worldPos;
  vec2 texCoord;
  flat uint materialIndex;
} fs_in;

layout(location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

float log10(float x)
{
  return log(x) / log(10.0);
}

float satf(float x)
{
  return clamp(x, 0.0, 1.0);
}

vec2 satv(vec2 x)
{
  return clamp(x, vec2(0.0), vec2(1.0));
}

float max2(vec2 v)
{
  return max(v.x, v.y);
}

ResolvedMaterial material;

void main()
{
  // const uint materialIndex = fs_in.materialIndex;

  // vec2 texCoord = fs_in.texCoord;
  // #include include/apply_parallax.glsl

  // #include include/var_tex_material.glsl

  // o_fragColor = vec4(material.diffuse.rgb, 0.7);

  // if (gl_FrontFacing) {
  //   o_fragColor.b = 1;
  // }
  // // o_fragColor = vec4(1, 0, 0, 1);

  const vec3 worldPos = fs_in.worldPos;
  const vec3 cameraWorldPos = u_cameraPos.xyz;

  const float gGridSize = 100.0;
  const float gGridMinPixelsBetweenCells = 2.0;
  const float gGridCellSize = 0.025;
  const vec4 gGridColorThin = vec4(0.5, 0.5, 0.5, 1.0);
  const vec4 gGridColorThick = vec4(0.1, 0.1, 0.2, 1.0);

  const vec2 dvx = vec2(dFdx(worldPos.x), dFdy(worldPos.x));
  const vec2 dvy = vec2(dFdx(worldPos.z), dFdy(worldPos.z));

  const float lx = length(dvx);
  const float ly = length(dvy);

  vec2 dudv = vec2(lx, ly);

  const float l = length(dudv);

  const float LOD = max(0.0, log10(l * gGridMinPixelsBetweenCells / gGridCellSize) + 1.0);

  const float gridCellSizeLod0 = gGridCellSize * pow(10.0, floor(LOD));
  const float gridCellSizeLod1 = gridCellSizeLod0 * 10.0;
  const float gridCellSizeLod2 = gridCellSizeLod1 * 10.0;

  dudv *= 4.0;

  vec2 mod_div_dudv = mod(worldPos.xz, gridCellSizeLod0) / dudv;
  const float Lod0a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

  mod_div_dudv = mod(worldPos.xz, gridCellSizeLod1) / dudv;
  const float Lod1a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

  mod_div_dudv = mod(worldPos.xz, gridCellSizeLod2) / dudv;
  const float Lod2a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

  const float LOD_fade = fract(LOD);
  vec4 color;

  if (Lod2a > 0.0) {
    color = gGridColorThick;
    color.a *= Lod2a;
  } else {
    if (Lod1a > 0.0) {
      color = mix(gGridColorThick, gGridColorThin, LOD_fade);
      color.a *= Lod1a;
    } else {
      color = gGridColorThin;
      color.a *= (Lod0a * (1.0 - LOD_fade));
    }
  }

  const float OpacityFalloff = 1.0 - satf(length(worldPos.xz - cameraWorldPos.xz) / gGridSize);

  color.a *= OpacityFalloff;

  o_fragColor = color;
}
