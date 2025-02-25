#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_debug.glsl
#include uniform_buffer_info.glsl

#include ssbo_materials.glsl

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
  flat vec3 normal;
  vec2 texCoord;

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
layout(binding = UNIT_G_NORMAL) uniform sampler2D g_normal;
layout(binding = UNIT_G_DEPTH) uniform sampler2D g_depth;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

// NOTE KI approx cos(90), NOT exact 0.0, due to small rounding errors in math
const float ANGLE_THREHOLD = 0.001;

ResolvedMaterial material;

#ifdef USE_PARALLAX
#include fn_calculate_parallax_mapping.glsl
#endif
#include fn_gbuffer_encode.glsl
#include fn_gbuffer_decode.glsl


void main() {
  const uint materialIndex = fs_in.materialIndex;

  // vec2 texCoord = fs_in.texCoord;
  // #include apply_parallax.glsl

  vec2 texCoord = fs_in.texCoord;

  vec3 worldPos;
  vec3 viewPos;
  vec3 oldNormal;
  float depth;

  if (!u_forceLineMode)
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

      oldNormal = decodeGNormal(pixCoord);
    }

    vec4 objectPos = fs_in.worlToLocalMatrix * vec4(worldPos, 1.0);
    texCoord = objectPos.xy + 0.5;

    if (abs(objectPos.x) >= 0.5 ||
	abs(objectPos.y) >= 0.5 ||
	abs(objectPos.z) >= 0.5)
    {
      discard;
      return;
    }

    vec3 normal = normalize(fs_in.normal);
    if (dot(normal, oldNormal) <= ANGLE_THREHOLD)
    {
      discard;
      return;
    }

    #include apply_parallax.glsl
  }

  #include var_tex_material.glsl

#ifdef USE_ALPHA
#ifdef USE_BLEND
  if (material.diffuse.a < OIT_MAX_BLEND_THRESHOLD) {
    discard;
  }
#else
  if (material.diffuse.a < GBUFFER_ALPHA_THRESHOLD) {
    discard;
  }
#endif
#endif

  #include var_tex_material_normal.glsl

  if (!gl_FrontFacing) {
    normal = -normal;
  }

#ifdef USE_CUBE_MAP
  const vec3 viewDir = normalize(u_cameraPos - fs_in.worldPos);

  #include var_calculate_cube_map_diffuse.glsl
#endif

  vec4 color = material.diffuse;

  clamp_color(color);

  o_fragColor = color.rgb;

  if (false && !u_forceLineMode) {
    // o_fragColor = worldPos * 0.1;
    // o_fragColor = vec3(worldPos.z);
    // o_fragColor = vec3(depth);
    // o_fragColor = vec3(objectPos.y + 807);
    // o_fragColor = vec3(objectPos.z);

    // float d2 = linearizeDepth(depth, u_nearPlane, u_farPlane);
    // o_fragColor.rgb = vec3(depth);
    // o_fragColor.rgb = vec3(texCoord.y);

    // o_fragColor = vec3(normal.y);
    // if (abs(oldNormal.x) > 0.9) {
    //   o_fragColor = vec3(1);
    // }

    float v = normal.x;
    if (v > 0.9) {
      o_fragColor = vec3(0, 1, 0);
    } else if (v < -0.9) {
      o_fragColor = vec3(0, 0, 1);
    }
  }

  if (u_forceLineMode) {
    o_fragColor = vec3(0, 1, 0);
  }

  o_fragMRA = material.mra;
  o_fragEmission = material.emission;

  #include encode_gbuffer_normal.glsl
}
