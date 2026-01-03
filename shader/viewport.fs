#version 460 core

#define EFF_NONE 0
#define EFF_INVERT 1
#define EFF_GRAY_SCALE 2
#define EFF_SHARPEN 3
#define EFF_BLUR 4
#define EFF_EDGE 5

#include "include/uniform_data.glsl"

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(binding = UNIT_VIEWPORT) uniform sampler2D u_viewportTex;

subroutine vec3 sub_effect(vec3 color);

layout(location = UNIFORM_BLEND_FACTOR) uniform float u_blendFactor;

layout(location = SUBROUTINE_EFFECT) subroutine uniform sub_effect u_effect;

layout(location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const float offset = 1.0 / 300.0;

layout (index = EFF_NONE)
subroutine (sub_effect)
vec3 effectNone(in vec3 color)
{
  return color;
}

layout (index = EFF_INVERT)
subroutine (sub_effect)
vec3 effectInvert(in vec3 color)
{
  return vec3(1.0 - color);
}

layout (index = EFF_GRAY_SCALE)
subroutine (sub_effect)
vec3 effectGrayScale(in vec3 color)
{
    float avg = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    return vec3(avg, avg, avg);
}

layout (index = EFF_SHARPEN)
subroutine (sub_effect)
vec3 effectSharpen(in vec3 color)
{
    vec2 offsets[9] = vec2[]
      (
       vec2(-offset,  offset), // top-left
       vec2( 0.0f,    offset), // top-center
       vec2( offset,  offset), // top-right
       vec2(-offset,  0.0f),   // center-left
       vec2( 0.0f,    0.0f),   // center-center
       vec2( offset,  0.0f),   // center-right
       vec2(-offset, -offset), // bottom-left
       vec2( 0.0f,   -offset), // bottom-center
       vec2( offset, -offset)  // bottom-right
       );

    float kernel[9] = float[]
      (
       -1, -1, -1,
       -1,  9, -1,
       -1, -1, -1
       );
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++) {
      sampleTex[i] = vec3(textureLod(u_viewportTex, fs_in.texCoord.st + offsets[i], 0));
    }

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
      col += sampleTex[i] * kernel[i];
    }
    return col;
}

layout (index = EFF_BLUR)
subroutine (sub_effect)
vec3 effectBlur(in vec3 color)
{
    vec2 offsets[9] = vec2[]
      (
       vec2(-offset,  offset), // top-left
       vec2( 0.0f,    offset), // top-center
       vec2( offset,  offset), // top-right
       vec2(-offset,  0.0f),   // center-left
       vec2( 0.0f,    0.0f),   // center-center
       vec2( offset,  0.0f),   // center-right
       vec2(-offset, -offset), // bottom-left
       vec2( 0.0f,   -offset), // bottom-center
       vec2( offset, -offset)  // bottom-right
       );

    float kernel[9] = float[]
      (
       1.0 / 16, 2.0 / 16, 1.0 / 16,
       2.0 / 16, 4.0 / 16, 2.0 / 16,
       1.0 / 16, 2.0 / 16, 1.0 / 16
       );

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++) {
      sampleTex[i] = vec3(textureLod(u_viewportTex, fs_in.texCoord.st + offsets[i], 0));
    }

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
      col += sampleTex[i] * kernel[i];
    }
    return col;
}

layout (index = EFF_EDGE)
subroutine (sub_effect)
vec3 effectEdge(in vec3 color)
{
    vec2 offsets[9] = vec2[]
      (
       vec2(-offset,  offset), // top-left
       vec2( 0.0f,    offset), // top-center
       vec2( offset,  offset), // top-right
       vec2(-offset,  0.0f),   // center-left
       vec2( 0.0f,    0.0f),   // center-center
       vec2( offset,  0.0f),   // center-right
       vec2(-offset, -offset), // bottom-left
       vec2( 0.0f,   -offset), // bottom-center
       vec2( offset, -offset)  // bottom-right
       );

    float kernel[9] = float[]
      (
       1.0,  1.0, 1.0,
       1.0, -8.0, 1.0,
       1.0,  1.0, 1.0
       );

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++) {
      sampleTex[i] = vec3(textureLod(u_viewportTex, fs_in.texCoord.st + offsets[i], 0));
    }

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
      col += sampleTex[i] * kernel[i];
    }
    return col;
}

/*
subroutine (sub_effect)
vec3 effectX(in vec3 color)
{
  vec3 color;
  if (u_effect == EFF_INVERT) {
    color = effectInvert(color);
  } else if (u_effect == EFF_GRAY_SCALE) {
    color = effectGrayScale(color);
  } else if (u_effect == EFF_SHARPEN) {
    color = effectSharpen(color);
  } else if (u_effect == EFF_BLUR) {
    color = effectBlur(color);
  } else if (u_effect == EFF_EDGE) {
    color = effectEdge(color);
  }
  return color;
}
*/

void main()
{
  vec4 orig = textureLod(u_viewportTex, fs_in.texCoord, 0);
  if (orig.a == 0) discard;

  vec3 color = u_effect(orig.rgb);

  float factor = u_blendFactor;
  float blend = orig.a * factor;

  o_fragColor = vec4(color, blend);
}
