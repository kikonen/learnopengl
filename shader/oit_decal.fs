#version 460 core

#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_data.glsl
#include uniform_debug.glsl
#include uniform_buffer_info.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  mat4 worlToLocalMatrix;

#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  flat vec2 spriteCoord;
  flat vec2 spriteSize;

  vec3 viewPos;
  flat vec3 decalNormal;

  flat uint materialIndex;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;
layout(binding = UNIT_G_DEPTH) uniform sampler2D g_depth;

LAYOUT_OIT_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_oit_util.glsl

// NOTE KI approx cos(90), NOT exact 0.0, due to small rounding errors in math
// cos(90) = 0
// cos(60) = 0.5
const float ANGLE_THREHOLD = 0.51;
const float DIM_THRESHOLD = 0.49;

ResolvedMaterial material;

#ifdef USE_PARALLAX
#include fn_calculate_parallax_mapping.glsl
#endif
#include fn_gbuffer_encode.glsl


void main() {
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord;
  vec3 worldPos;
  vec3 viewPos;
  vec3 surfaceNormal;
  float depth;

  {
    {
      vec2 pixCoord = gl_FragCoord.xy / u_bufferResolution;
      depth = textureLod(g_depth, pixCoord, 0).x;

      const vec4 clip = vec4(
	pixCoord.x * 2.0 - 1.0,
	pixCoord.y * 2.0 - 1.0,
	depth * 2.0 - 1.0,
	1.0);
      vec4 viewW  = u_invProjectionMatrix * clip;
      viewPos  = viewW.xyz / viewW.w;
      worldPos = (u_invViewMatrix * vec4(viewPos, 1)).xyz;

      // NOTE KI *MUST* calculate from depth, since g_normal
      // may point into *ANY* direction, thus checking against it
      // cannot work as expected
      vec3 ddxWp = dFdx(worldPos);
      vec3 ddyWp = dFdy(worldPos);
      surfaceNormal = normalize(cross(ddxWp, ddyWp));
    }

    vec4 objectPos = fs_in.worlToLocalMatrix * vec4(worldPos, 1.0);
    texCoord = objectPos.xy + 0.5;

    if (abs(objectPos.x) >= DIM_THRESHOLD ||
	abs(objectPos.y) >= DIM_THRESHOLD ||
	abs(objectPos.z) >= DIM_THRESHOLD)
    {
      discard;
      return;
    }

    if (dot(fs_in.decalNormal, surfaceNormal) <= ANGLE_THREHOLD)
    {
      discard;
      return;
    }

    #include apply_parallax.glsl
  }

  texCoord.x *= u_materials[materialIndex].tilingX;
  texCoord.y *= u_materials[materialIndex].tilingY;

  texCoord.x = fs_in.spriteCoord.x + texCoord.x * fs_in.spriteSize.x;
  texCoord.y = fs_in.spriteCoord.y + texCoord.y * fs_in.spriteSize.y;

  #include var_tex_material.glsl

  OIT_DISCARD(material.diffuse.alpha);

  vec3 normal = surfaceNormal;

  #include apply_normal_map.glsl

  if (!gl_FrontFacing) {
    normal = -normal;
  }

#ifdef USE_CUBE_MAP
  const vec3 viewDir = normalize(u_cameraPos.xyz - fs_in.worldPos);

  #include var_calculate_cube_map_diffuse.glsl
#endif

  vec4 color = material.diffuse;

  clamp_color(color);

  o_fragColor = color.rgb;

  #include encode_gbuffer_normal.glsl
}
