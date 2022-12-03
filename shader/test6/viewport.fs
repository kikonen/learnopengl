#version 450 core
const int EFF_NONE = 0;
const int EFF_INVERT = 1;
const int EFF_GRAY_SCALE = 2;
const int EFF_SHARPEN = 3;
const int EFF_BLUR = 4;
const int EFF_EDGE = 5;

out vec4 fragColor;

in vec2 texCoord;

layout(binding = UNIT_VIEWPORT) uniform sampler2D u_viewportTex;

uniform int u_effect;

const float offset = 1.0 / 300.0;

precision mediump float;

void main()
{
  vec4 color = texture(u_viewportTex, texCoord);

  if (u_effect == EFF_INVERT) {
    color = vec4(vec3(1.0 - color), color.a);
  } else if (u_effect == EFF_GRAY_SCALE) {
    float avg = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    color = vec4(avg, avg, avg, color.a);
  } else if (u_effect == EFF_SHARPEN) {
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
      sampleTex[i] = vec3(texture(u_viewportTex, texCoord.st + offsets[i]));
    }

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
      col += sampleTex[i] * kernel[i];
    }
    color = vec4(col, 1.0);
  } else if (u_effect == EFF_BLUR) {
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
      sampleTex[i] = vec3(texture(u_viewportTex, texCoord.st + offsets[i]));
    }

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
      col += sampleTex[i] * kernel[i];
    }
    color = vec4(col, 1.0);
  } else if (u_effect == EFF_EDGE) {
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
      sampleTex[i] = vec3(texture(u_viewportTex, texCoord.st + offsets[i]));
    }

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
      col += sampleTex[i] * kernel[i];
    }
    color = vec4(col, 1.0);
  }

  fragColor = color;
}
