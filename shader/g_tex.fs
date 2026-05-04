#version 460 core

#include "include/ssbo_materials.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"
#include "include/uniform_buffer_info.glsl"
#include "include/uniform_debug.glsl"

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
  flat uint flags;

#ifdef USE_TBN
  vec4 tangent;
#endif

#ifdef USE_JOINTS
#ifdef USE_DEBUG
  flat uint boneBaseIndex;
  flat uvec4 boneIndex;
  flat vec4 boneWeight;
  vec3 boneColor;
#endif
#endif
// #ifdef USE_DEBUG
//   flat uint socketIndex;
// #endif

#ifdef USE_WIREFRAME_MOD
  noperspective vec3 edgeDistance;
#endif
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#ifdef USE_PARALLAX
#include "include/fn_calculate_parallax_mapping.glsl"
#endif
#include "include/fn_gbuffer_normal_encode.glsl"

void main() {
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;

  // NOTE KI interpolation from vs to fs denormalizes normal.
  // Declared early so var_calculate_tbn.glsl (and the old-path apply_parallax)
  // can both consume the same oriented normal.
  vec3 normal = normalize(fs_in.normal);
  if (!gl_FrontFacing) {
    normal = -normal;
  }

  // NOTE KI reconstruct tangent basis in FS to keep normal mapping
  // and parallax on the same basis (avoids interpolation drift
  // between VS-computed tangentPos and FS-computed tbn).
  #include "include/var_calculate_tbn.glsl"

  #include "include/apply_parallax.glsl"

  #include "include/var_tex_material.glsl"

  // NOTE KI alpha/blend does not co-op with line mode
  if (!u_forceLineMode) {
#ifdef USE_ALPHA
#ifdef USE_BLEND
  if (material.diffuse.a < u_oitMaxBlendThreshold) {
      discard;
  }
#else
  if (material.diffuse.a < GBUFFER_ALPHA_THRESHOLD)
    discard;
#endif
#endif
  }

  #include "include/apply_normal_map.glsl"

#ifdef USE_CUBE_MAP
  {
    const vec3 viewDir = -normalize(fs_in.viewPos);
#include "include/var_calculate_cube_map_diffuse.glsl"
  }
#endif

  vec4 color = material.diffuse;

  clamp_color(color);

#ifdef TOY
  vec4 fragColor = vec4(0);
  mainImage(fragColor, u_bufferResolution, u_time, gl_FragCoord.xy);
  if (fragColor.a < 0.1) {
    //discard;
  } else {
    color = fragColor;
  }
#endif

#ifdef USE_WIREFRAME_MOD
  vec3 saveColor = color.rgb;

  float d = min(
    min(
      fs_in.edgeDistance.x,
      fs_in.edgeDistance.y),
    fs_in.edgeDistance.z);

  float lineWidth = Debug.u_wireframeLineWidth;
  vec3 lineColor = Debug.u_wireframeLineColor.rgb;
  // lineWidth = 0.5;

  float mixVal = smoothstep(lineWidth - 1, lineWidth + 1, d);
  color.rgb = mix(lineColor.rgb, saveColor.rgb, mixVal);

  if (!u_forceLineMode) {
#ifdef USE_ALPHA
  if (Debug.u_wireframeOnly && mixVal > 0.9) discard;
  // color.rgb = saveColor.rgb;
#endif
  }
#endif

  // if (!gl_FrontFacing) {
  //   float alpha = texColor.a;
  //   texColor = mix(texColor, vec4(0.1, 0.1, 0.9, 1.0), 0.15);
  //   texColor.a = alpha;
  // }

  if (u_waterCausticMaterialIndex > 0) {
    vec2 causticTexCoord = (texCoord + vec2(sin(u_time * 0.2), cos(u_time * 0.1)) * 0.3) * 1.5;
    vec3 causticColor = texture(sampler2D(u_materials[u_waterCausticMaterialIndex].diffuseTex), causticTexCoord).rgb;

    color.rgb = mix(color.rgb, causticColor.rgb, 0.2);
  }

  o_fragColor = color.rgb;
  if (u_forceLineMode) {
    o_fragColor = vec3(0, 0, 1);
  }

  o_fragMRAS = material.mras;
  o_fragEmission = material.emission;

#ifdef USE_JOINTS
  // o_fragColor = vec3(1.0, 0.0, 0.0);

  // if (fs_in.boneIndex.x > 150) {
  //   o_fragColor = vec3(0.0, 0.0, 1.0);
  // }
#ifdef USE_DEBUG
  if (Debug.u_boneWeight) {
    vec3 c = fs_in.boneColor;
    float sum = c.x + c.y + c.z;
    if (sum > 0) {
      o_fragColor = fs_in.boneColor;
    }
  }
#endif
#endif

// #ifdef USE_DEBUG
//   if (fs_in.socketIndex > 0) {
//     o_fragColor = vec3(1, 1, 0);
//   }
// #endif

  #include "include/encode_gbuffer_normal.glsl"
  #include "include/encode_gbuffer_view_position.glsl"
  #include "include/encode_gbuffer_view_z.glsl"
}
