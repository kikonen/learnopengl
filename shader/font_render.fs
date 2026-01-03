#version 460 core

#include "include/ssbo_materials.glsl"


in VS_OUT {
  vec2 texCoord;

  vec2 atlasCoord;
  flat uvec2 atlasHandle;

  flat uint materialIndex;
} fs_in;


layout(binding = UNIT_FONT_ATLAS) uniform sampler2D u_fontAtlas;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

void main()
{
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;
  #include "include/apply_parallax.glsl"

  #include "include/var_tex_material.glsl"

  vec4 color;
  shapeFont(fs_in.atlasHandle, fs_in.atlasCoord, true, color);

#ifdef USE_ALPHA
#ifdef USE_BLEND
#else
  if (color.a < GBUFFER_ALPHA_THRESHOLD)
    discard;
#endif
#endif

  o_fragColor = color;
}
