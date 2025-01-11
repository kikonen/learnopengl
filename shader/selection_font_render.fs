#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl
#include uniform_data.glsl

#include ssbo_materials.glsl

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

#include fn_shape_font.glsl

void main()
{
  material.diffuse = vec4(1, 1, 1, 1);

  vec4 color;
  shapeFont(fs_in.atlasHandle, fs_in.atlasCoord, false, color);

  if (color.a < 0.65)
    discard;

  const uint materialIndex = fs_in.highlightIndex;
  o_fragColor = u_materials[materialIndex].diffuse;
}
