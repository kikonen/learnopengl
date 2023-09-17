#version 460 core

in VS_OUT {
  vec3 worldPos;
} fs_in;

layout(binding = UNIT_SKYBOX) uniform samplerCube u_skybox;

out vec4 u_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const float PI = 3.14159265359;

const float MIN_COL_VALUE = 0.0;
const float MAX_COL_VALUE = 400.0;

const float MIN_IRR_VALUE = 0.0;
const float MAX_IRR_VALUE = 100.0;

void main()
{
  // the sample direction equals the hemisphere's orientation
  vec3 normal = normalize(fs_in.worldPos);

  vec3 irradiance = vec3(0.0);

  vec3 up    = vec3(0.0, 1.0, 0.0);
  vec3 right = normalize(cross(up, normal));
  up         = normalize(cross(normal, right));

  float sampleDelta = 0.025;
  float nrSamples = 0.0;
  for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
    for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
      // spherical to cartesian (in tangent space)
      vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
      // tangent space to world
      vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

      vec3 color = texture(u_skybox, sampleVec).rgb;
      color.r = clamp(color.r, MIN_COL_VALUE, MAX_COL_VALUE);
      color.g = clamp(color.g, MIN_COL_VALUE, MAX_COL_VALUE);
      color.b = clamp(color.b, MIN_COL_VALUE, MAX_COL_VALUE);

      irradiance += color * cos(theta) * sin(theta);
      nrSamples++;
    }
  }

  irradiance = PI * irradiance * (1.0 / float(nrSamples));

  irradiance.r = clamp(irradiance.r, MIN_IRR_VALUE, MAX_IRR_VALUE);
  irradiance.g = clamp(irradiance.g, MIN_IRR_VALUE, MAX_IRR_VALUE);
  irradiance.b = clamp(irradiance.b, MIN_IRR_VALUE, MAX_IRR_VALUE);

  u_fragColor = vec4(irradiance, 1.0);
}
