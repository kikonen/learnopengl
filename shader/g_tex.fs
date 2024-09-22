#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_buffer_info.glsl
#include uniform_debug.glsl
#include ssbo_materials.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
  flat uint flags;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif

#ifdef USE_BONES
#ifdef USE_DEBUG
  flat uint boneBaseIndex;
  flat uvec4 boneIndex;
  flat vec4 boneWeight;
  vec3 boneColor;
#endif
#endif
#ifdef USE_DEBUG
  flat uint socketBaseIndex;
  flat uint socketIndex;
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
#include fn_calculate_parallax_mapping.glsl
#endif
#include fn_gbuffer_encode.glsl

#ifdef TOY
// shading 3D sphere
void mainImage(
  out vec4 fragColor,
  in vec2 iResolution,
  in float iTime,
  in vec2 fragCoord)
{
  // normailze and adjsut for ratio
  vec2 res = iResolution.xy;
  vec2 uv = (fragCoord*2.0-res ) / res.y;
  uv*=.7;
  //initilize colors
  vec4 color1 = vec4(.4,.6,.7,1.0);
  vec4 color2 = vec4(.9,.7,.6,1.0);

  // shade with 2 faux lights
  color1*=.8-distance(uv,vec2(-.1,-.1));
  color2*=.6-distance(uv,vec2(.25,.3));
  vec4 sphere = color1+color2 ;

  //limit edges to circle shape
  float d = distance(uv, vec2(0.0));
  // smooth edges
  float t =1.0- smoothstep(.6,.61, d);
  // apply shape to colors
  sphere*=t+sin(iTime)*.2*uv.y;

  //output final color, and brighten
  fragColor = sphere*1.6;
}
#endif

void main() {
  const uint materialIndex = fs_in.materialIndex;

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

#ifdef USE_ALPHA
#ifdef USE_BLEND
  if (material.diffuse.a < 0.7)
    discard;
#else
  if (material.diffuse.a < 0.05)
    discard;
#endif
#endif

  #include var_tex_material_normal.glsl

  if (!gl_FrontFacing) {
    normal = -normal;
  }

#ifdef USE_CUBE_MAP
  const vec3 viewDir = normalize(u_viewWorldPos - fs_in.worldPos);

  #include var_calculate_cube_map_diffuse.glsl
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

  // if (!gl_FrontFacing) {
  //   float alpha = texColor.a;
  //   texColor = mix(texColor, vec4(0.1, 0.1, 0.9, 1.0), 0.15);
  //   texColor.a = alpha;
  // }

  o_fragColor = color.rgb;
  o_fragMetal = material.metal;
  o_fragEmission = material.emission.rgb;

#ifdef USE_BONES
  // o_fragColor = vec3(1.0, 0.0, 0.0);

  // if (fs_in.boneIndex.x > 150) {
  //   o_fragColor = vec3(0.0, 0.0, 1.0);
  // }
#ifdef USE_DEBUG
  if (u_debugBoneWeight) {
    vec3 c = fs_in.boneColor;
    float sum = c.x + c.y + c.z;
    if (sum > 0) {
      o_fragColor = fs_in.boneColor;
    }
  }
#endif
#endif

#ifdef USE_PARALLAX
  if (material.metal.b > 0) {
    // o_fragColor = vec3(material.metal.b, 0, 0);
  }
#endif

// #ifdef USE_DEBUG
//   if (fs_in.socketBaseIndex + fs_in.socketIndex > 0) {
//     o_fragColor = vec3(1, 1, 0);
//   }
// #endif

  //o_fragPosition = fs_in.worldPos;
  o_fragNormal = encodeGNormal(normal, fs_in.viewPos);
}
