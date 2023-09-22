#version 460 core
in vec3 texCoord;

layout(early_fragment_tests) in;

#include uniform_data.glsl

layout(binding = UNIT_SKYBOX) uniform samplerCube u_skybox;
layout(binding = UNIT_IRRADIANCE_MAP) uniform samplerCube u_irradianceMap;
layout(binding = UNIT_PREFILTER_MAP) uniform samplerCube u_prefilterMap;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_gbuffer_encode.glsl

void main() {
  vec3 envColor = texture(u_skybox, texCoord).rgb;
  vec3 irradianceColor = texture(u_irradianceMap, texCoord).rgb;
  vec3 prefilterColor = textureLod(u_prefilterMap, texCoord, 0).rgb;

  //texColor = vec4(1.0, 0, 0, 1.0);
  //texColor.a = 0.0;

  vec3 texColor = envColor;
  texColor = mix(texColor, irradianceColor, 0.0);
  texColor = mix(texColor, prefilterColor, 0.0);

  o_fragColor = vec4(texColor.rgb, 1.0);
  //o_fragSpecular = vec4(0);
  o_fragMetal = vec4(0);
  o_fragEmission = vec3(0);

  //o_fragPosition = u_viewWorldPos + 1000 * u_viewFront;
  o_fragNormal = encodeGNormal(u_viewFront);
}
