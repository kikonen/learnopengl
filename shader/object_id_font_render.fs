#version 460 core

#include include/ssbo_materials.glsl

#include include/uniform_data.glsl

in VS_OUT {
  flat vec4 objectID;

  vec2 texCoord;

  vec2 atlasCoord;
  flat uvec2 atlasHandle;

  flat uint materialIndex;
  flat uint flags;
} fs_in;


layout (location = 0) out vec4 o_fragObjectID;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

ResolvedMaterial material;

#include include/fn_shape_font.glsl

void main() {
  {
    const vec2 texCoord = fs_in.texCoord;
    #include include/var_tex_material_alpha.glsl

    material.diffuse = vec4(1, 1, 1, 1);

    vec4 color;
    shapeFont(fs_in.atlasHandle, fs_in.atlasCoord, false, color);
    alpha = color.a;

    // NOtE KI experimental value; depends from few aspects in blended windows
    // NOTE KI this works badly for blended objects if threshold too big
    if (alpha < 0.1)
      discard;
  }

  o_fragObjectID = fs_in.objectID;
}
