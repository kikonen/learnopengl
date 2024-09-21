#version 460 core

#define EFF_NONE 0
#define EFF_INVERT 1
#define EFF_GRAY_SCALE 2
#define EFF_SHARPEN 3
#define EFF_BLUR 4
#define EFF_EDGE 5

#include uniform_data.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in vec2 texCoord;

layout(binding = UNIT_VIEWPORT) uniform sampler2D u_viewportTex;

subroutine vec4 sub_effect(vec4 color);

layout(location = UNIFORM_TONE_HDRI) uniform bool u_toneHdri;
layout(location = UNIFORM_GAMMA_CORRECT) uniform bool u_gammaCorrect;
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
vec4 effectNone(in vec4 color)
{
  return color;
}

layout (index = EFF_INVERT)
subroutine (sub_effect)
vec4 effectInvert(in vec4 color)
{
  return vec4(vec3(1.0 - color), color.a);
}

layout (index = EFF_GRAY_SCALE)
subroutine (sub_effect)
vec4 effectGrayScale(in vec4 color)
{
    float avg = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    return vec4(avg, avg, avg, color.a);
}

layout (index = EFF_SHARPEN)
subroutine (sub_effect)
vec4 effectSharpen(in vec4 color)
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
      sampleTex[i] = vec3(textureLod(u_viewportTex, texCoord.st + offsets[i], 0));
    }

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
      col += sampleTex[i] * kernel[i];
    }
    return vec4(col, 1.0);
}

layout (index = EFF_BLUR)
subroutine (sub_effect)
vec4 effectBlur(in vec4 color)
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
      sampleTex[i] = vec3(textureLod(u_viewportTex, texCoord.st + offsets[i], 0));
    }

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
      col += sampleTex[i] * kernel[i];
    }
    return vec4(col, 1.0);
}

layout (index = EFF_EDGE)
subroutine (sub_effect)
vec4 effectEdge(in vec4 color)
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
      sampleTex[i] = vec3(textureLod(u_viewportTex, texCoord.st + offsets[i], 0));
    }

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
      col += sampleTex[i] * kernel[i];
    }
    return vec4(col, 1.0);
}

/*
subroutine (sub_effect)
vec4 effectX(in vec4 color)
{
  vec4 color;
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
  vec3 color = u_effect(textureLod(u_viewportTex, texCoord, 0)).rgb;

  if (u_toneHdri) {
    // reinhard
    // vec3 result = hdrColor / (hdrColor + vec3(1.0));
    // exposure
    color = vec3(1.0) - exp(-color * u_hdrExposure);
  }

  if (u_gammaCorrect) {
    // also gamma correct while we're at it
    color = pow(color, vec3(1.0 / u_hdrGamma));
  }

  o_fragColor = vec4(color, u_blendFactor);
}
