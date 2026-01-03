#version 460 core

#include "include/ssbo_materials.glsl"

#include "include/uniform_data.glsl"

in VS_OUT {
  vec2 texCoord;

  vec2 atlasCoord;
  flat uvec2 atlasHandle;

  flat uint materialIndex;
} fs_in;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include "include/fn_shape_font.glsl"

void main()
{
  {
    const uint materialIndex = fs_in.materialIndex;

    const vec2 texCoord = fs_in.texCoord;
    #include "include/var_tex_material_alpha.glsl"

    // NOtE KI experimental value; depends from few aspects in blended windows
    if (alpha < SHADOW_ALPHA_THRESHOLD)
      discard;
  }

  material.diffuse = vec4(1, 1, 1, 1);

  vec4 color;
  shapeFont(fs_in.atlasHandle, fs_in.atlasCoord, false, color);

  if (color.a < 0.65)
    discard;
}
