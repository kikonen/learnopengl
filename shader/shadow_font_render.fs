#version 460 core

#include struct_resolved_material.glsl

in VS_OUT {
  vec2 atlasCoord;
  flat uvec2 atlasHandle;
} fs_in;

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
  shapeFont(fs_in.atlasHandle, fs_in.atlasCoord, color);

  if (color.a < 0.65)
    discard;
}
