#version 460 core

#include "include/ssbo_materials.glsl"

#include "include/uniform_data.glsl"

in VS_OUT {
  vec2 atlasCoord;
  flat uvec2 atlasHandle;
  flat uint highlightIndex;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include "include/fn_shape_font.glsl"

void main()
{
  material.diffuse = vec4(1, 1, 1, 1);

  vec4 color;
  bool blend = false;
#ifdef USE_BLEND
  blend = true;
#endif
  shapeFont(fs_in.atlasHandle, fs_in.atlasCoord, blend, color);

  if (color.a < 0.01)
    discard;

  const uint materialIndex = fs_in.highlightIndex;
  o_fragColor = u_materials[materialIndex].diffuse;
}
