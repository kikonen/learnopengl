#define _UBO_DATA
layout(std140, binding = UBO_DATA) uniform Data {
  vec4 u_fogColor;

  uint u_selectionMaterialIndex;
  uint u_tagMaterialIndex;

  bool u_cubeMapEnabled;

  // TODO KI NOT USED
  bool u_skyboxExist;

  // TODO KI NOT USED
  bool u_environmentMapExist;

  bool u_shadowVisual;
  bool u_forceLineMode;

  float u_fogStart;
  float u_fogEnd;
  float u_fogDensity;

  float u_oitMinBlendThreshold;
  float u_oitMaxBlendThreshold;

  float u_bloomThreshold;

  float u_gammaCorrect;
  float u_hdrExposure;

  float u_time;
  int u_frame;
  int u_shadowCount;

  vec4 u_ssaoSamples[64];

  float u_shadowCascade_0;
  float u_shadowCascade_1;
  float u_shadowCascade_2;
  float u_shadowCascade_3;

  // int pad1;
  // int pad2;
};
