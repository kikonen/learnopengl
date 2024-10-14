#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include uniform_data.glsl
#include ssbo_materials.glsl

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
    float f = log(x) / log(10.0);
    return f;
}


float satf(float x)
{
    float f = clamp(x, 0.0, 1.0);
    return f;
}


vec2 satv(vec2 x)
{
    vec2 v = clamp(x, vec2(0.0), vec2(1.0));
    return v;
}


float max2(vec2 v)
{
    float f = max(v.x, v.y);
    return f;
}

ResolvedMaterial material;

void main()
{
  // const uint materialIndex = fs_in.materialIndex;

  // #include var_tex_coord.glsl
  // #include var_tex_material.glsl

  // o_fragColor = vec4(material.diffuse.rgb, 0.7);

  // if (gl_FrontFacing) {
  //   o_fragColor.b = 1;
  // }
  // // o_fragColor = vec4(1, 0, 0, 1);

  const vec3 WorldPos = fs_in.worldPos;
  const vec3 gCameraWorldPos = u_viewWorldPos.xyz;

  const float gGridSize = 100.0;
  const float gGridMinPixelsBetweenCells = 2.0;
  const float gGridCellSize = 0.025;
  const vec4 gGridColorThin = vec4(0.5, 0.5, 0.5, 1.0);
  const vec4 gGridColorThick = vec4(0.0, 0.0, 0.0, 1.0);

  const vec2 dvx = vec2(dFdx(WorldPos.x), dFdy(WorldPos.x));
  const vec2 dvy = vec2(dFdx(WorldPos.z), dFdy(WorldPos.z));

  const float lx = length(dvx);
  const float ly = length(dvy);

  vec2 dudv = vec2(lx, ly);

  const float l = length(dudv);

  const float LOD = max(0.0, log10(l * gGridMinPixelsBetweenCells / gGridCellSize) + 1.0);

  const float GridCellSizeLod0 = gGridCellSize * pow(10.0, floor(LOD));
  const float GridCellSizeLod1 = GridCellSizeLod0 * 10.0;
  const float GridCellSizeLod2 = GridCellSizeLod1 * 10.0;

  dudv *= 4.0;

  vec2 mod_div_dudv = mod(WorldPos.xz, GridCellSizeLod0) / dudv;
  const float Lod0a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

  mod_div_dudv = mod(WorldPos.xz, GridCellSizeLod1) / dudv;
  const float Lod1a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

  mod_div_dudv = mod(WorldPos.xz, GridCellSizeLod2) / dudv;
  const float Lod2a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)) );

  const float LOD_fade = fract(LOD);
  vec4 Color;

  if (Lod2a > 0.0) {
    Color = gGridColorThick;
    Color.a *= Lod2a;
  } else {
    if (Lod1a > 0.0) {
      Color = mix(gGridColorThick, gGridColorThin, LOD_fade);
      Color.a *= Lod1a;
    } else {
      Color = gGridColorThin;
      Color.a *= (Lod0a * (1.0 - LOD_fade));
    }
  }

  const float OpacityFalloff = (1.0 - satf(length(WorldPos.xz - gCameraWorldPos.xz) / gGridSize));

  Color.a *= OpacityFalloff;

  o_fragColor = Color;
}
