#version 460 core

#include "include/tbo_materials.glsl"

in VS_OUT {
  vec2 texCoord;

  vec2 atlasCoord;
  flat uvec2 atlasHandle;

  flat uint materialIndex;
} fs_in;


layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include "include/fn_fill_material_tbo.glsl"
#include "include/fn_shape_font.glsl"

void main()
{
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;
  #include "include/apply_parallax_tbo.glsl"
  fillMaterialTBO(materialIndex, texCoord);

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
