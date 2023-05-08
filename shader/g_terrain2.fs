#version 460 core

#include struct_material.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in TES_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif

  float height;
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

layout (location = 0) out vec4 o_fragColor;
layout (location = 1) out vec4 o_fragSpecular;
layout (location = 2) out vec4 o_fragEmission;
layout (location = 3) out vec3 o_fragPosition;
layout (location = 4) out vec3 o_fragNormal;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION

#include fn_calculate_normal_pattern.glsl

void main() {
  const vec3 toView = normalize(u_viewWorldPos - fs_in.worldPos);
  const Entity entity = u_entities[fs_in.entityIndex];
  #include var_tex_material.glsl

  #include var_tex_material_normal.glsl

  if (material.pattern == 1) {
    normal = calculateNormalPattern(fs_in.vertexPos, normal);
  }

  // if (!gl_FrontFacing) {
  //   normal = -normal;
  // }

  #include var_calculate_cubemap_diffuse.glsl

  vec4 texColor = material.diffuse;

  o_fragColor = vec4(texColor.xyz, material.ambient);
  o_fragSpecular = material.specular;
  o_fragSpecular.a = material.shininess;
  o_fragEmission = material.emission;

  o_fragPosition = fs_in.worldPos;
  o_fragNormal = normal;
}
