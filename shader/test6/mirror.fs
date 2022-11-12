#version 450 core

#include constants.glsl

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

in VS_OUT {
  vec4 glp;

  vec3 fragPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewVertexPos;

  flat uint materialIndex;

  vec4 fragPosLightSpace;

  mat3 TBN;
} fs_in;

uniform sampler2D u_reflectionTex;

uniform sampler2DShadow u_shadowMap;

layout (location = 0) out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision lowp float;

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_normal_pattern.glsl
#include fn_calculate_fog.glsl

void main() {
  #include var_tex_material.glsl

  vec3 normal = fs_in.normal;

#ifdef USE_RENDER_BACK
  if (!gl_FrontFacing) {
    normal = -normal;
  }
#endif

  vec3 toView = normalize(u_viewPos - fs_in.fragPos);

#ifdef USE_RENDER_BACK
  if (gl_FrontFacing)
#endif
  {
    vec4 gp = fs_in.glp;
    vec2 reflectCoord = vec2(-gp.x, gp.y) / (gp.w * 2.0) + 0.5;

    vec4 reflectColor = texture(u_reflectionTex, reflectCoord);

    vec4 mixColor = reflectColor;

    vec4 origDiffuse = material.diffuse;
    material.diffuse = mix(material.diffuse, mixColor, 0.9);
  }

  vec4 shaded = calculateLight(normal, toView, material);
  vec4 texColor = shaded;
  texColor = calculateFog(material.fogRatio, texColor);

  fragColor = vec4(texColor.xyz, 1.0);
}
