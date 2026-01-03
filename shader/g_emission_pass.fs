#version 460 core

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
// NOTE KI "early_fragment_tests" cannot be used at same same with alpha
// => disables "discard" logic
//layout(early_fragment_tests) in;

#include "include/screen_tri_vertex_out.glsl"

layout(binding = UNIT_G_EMISSION) uniform sampler2D g_emission;
layout(binding = UNIT_G_DEPTH) uniform sampler2D g_depth;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

vec4 calculateEmission(vec2 texCoord)
{
  const vec2 ts = vec2(0.001, 0.001);

  vec2 offsets[9] = vec2[]
      (
       vec2(-ts.x,  ts.y), // top-left
       vec2( 0.0f,    ts.y), // top-center
       vec2( ts.x,  ts.y), // top-right
       vec2(-ts.x,  0.0f),   // center-left
       vec2( 0.0f,    0.0f),   // center-center
       vec2( ts.x,  0.0f),   // center-right
       vec2(-ts.x, -ts.y), // bottom-left
       vec2( 0.0f,   -ts.y), // bottom-center
       vec2( ts.x, -ts.y)  // bottom-right
       );

    // float kernel[9] = float[]
    //   (
    //    1.0 / 16, 2.0 / 16, 1.0 / 16,
    //    2.0 / 16, 4.0 / 16, 2.0 / 16,
    //    1.0 / 16, 2.0 / 16, 1.0 / 16
    //    );

    float kernel[9] = float[]
      (
       0.0, 1.0, 0.0,
       1.0, 3.0, 1.0,
       0.0, 1.0, 0.0
       );

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++) {
      sampleTex[i] = vec3(texture(g_emission, texCoord + offsets[i]));
    }

    float phase = sin(u_time * 0.5) * 0.5 + 0.5;

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
      col += sampleTex[i] * kernel[i] * (0.2 + phase * 0.6);
    }
    return vec4(col, 0.4 + phase * 0.4);
}

void main()
{
  #include "include/screen_tri_tex_coord.glsl"

  vec4 color = calculateEmission(texCoord);

  if (color.r <= 0.01 && color.g <= 0.01 && color.b <= 0.01) {
    discard;
  }

  o_fragColor = color;
}
