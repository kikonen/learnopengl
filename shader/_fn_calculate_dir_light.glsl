
float lookup(
  in uint shadowIndex,
  in vec4 shadowPos,
  in float x,
  in float y,
  in float bias)
{
  // NOtE KI textureProj == automatic p.xyz / p.w
  float t = textureProj(u_shadowMap[shadowIndex],
                        shadowPos + vec4(x * 0.001 * shadowPos.w,
                                         y * 0.001 * shadowPos.w,
                                         -0.01,
                                         0.0),
                        bias);
  return t;
}

float calcShadow(
  in uint shadowIndex,
  in vec4 shadowPos,
  in vec3 normal,
  in vec3 toLight)
{
  float bias = max(0.05 * (1.0 - dot(normal, toLight)), 0.005);

  float swidth = 0.5;
  vec2 o = mod(floor(gl_FragCoord.xy), 2.0) * swidth;

  float d1 = 0.5 * swidth;
  float d2 = 1.5 * swidth;

  float shadowFactor = 0.0;
  shadowFactor += lookup(shadowIndex, shadowPos, -d2 + o.x,  d2 - o.y, bias);
  shadowFactor += lookup(shadowIndex, shadowPos, -d2 + o.x, -d1 - o.y, bias);
  shadowFactor += lookup(shadowIndex, shadowPos,  d1 + o.x,  d2 - o.y, bias);
  shadowFactor += lookup(shadowIndex, shadowPos,  d1 + o.x, -d1 - o.y, bias);
  shadowFactor = shadowFactor / 4.0;

  return shadowFactor;
}

float calcShadow2(
  in uint shadowIndex,
  in vec4 shadowPos,
  in vec3 normal,
  in vec3 toLight)
{
  float bias = max(0.05 * (1.0 - dot(normal, toLight)), 0.005);
  // NOtE KI textureProj == automatic p.xyz / p.w
  return textureProj(u_shadowMap[shadowIndex], shadowPos, bias);
}

vec2 poissonDisk[16] = vec2[](
   vec2( -0.94201624, -0.39906216 ),
   vec2( 0.94558609, -0.76890725 ),
   vec2( -0.094184101, -0.92938870 ),
   vec2( 0.34495938, 0.29387760 ),
   vec2( -0.91588581, 0.45771432 ),
   vec2( -0.81544232, -0.87912464 ),
   vec2( -0.38277543, 0.27676845 ),
   vec2( 0.97484398, 0.75648379 ),
   vec2( 0.44323325, -0.97511554 ),
   vec2( 0.53742981, -0.47373420 ),
   vec2( -0.26496911, -0.41893023 ),
   vec2( 0.79197514, 0.19090188 ),
   vec2( -0.24188840, 0.99706507 ),
   vec2( -0.81409955, 0.91437590 ),
   vec2( 0.19984126, 0.78641367 ),
   vec2( 0.14383161, -0.14100790 )
);

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i)
{
  vec4 seed4 = vec4(seed, i);
  float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
  return fract(sin(dot_product) * 43758.5453);
}

// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
float calcShadow2_3(
  in vec3 worldPos,
  in uint shadowIndex,
  in vec4 shadowPos,
  in vec3 normal,
  in vec3 toLight)
{
  // NOtE KI textureProj == automatic p.xyz / p.w

  if (shadowPos.z > 1.0) return 0.0;

  const float shadowDepth = shadowPos.z;

  //float bias = max(0.03 * (1.0 - dot(normal, toLight)), 0.005);

  // cosTheta is dot( n,l ), clamped between 0 and 1
  float cosTheta = clamp(dot(normal, toLight), 0.0, 1.0);
  float bias = 0.005 * tan(acos(cosTheta));
  bias = clamp(bias, 0.0, 0.01);

  float visibility = 1.0;
  for (int i = 0; i < 4; i++) {
    // use either :
    //  - Always the same samples.
    //    Gives a fixed pattern in the shadow, but no noise
    int index = i;
    //  - A random sample, based on the pixel's screen location.
    //    No banding, but the shadow moves with the camera, which looks weird.
    // int index = int(16.0*random(gl_FragCoord.xyy, i))%16;
    //  - A random sample, based on the pixel's position in world space.
    //    The position is rounded to the millimeter to avoid too much aliasing
    //int index = int(16.0 * random(floor(worldPos.xyz * 1000.0), i)) % 16;

    visibility -= 0.2 * (1.0 - texture(
                           u_shadowMap[shadowIndex],
                           vec3(
                             shadowPos.xy + poissonDisk[index] / 700.0,
                             (shadowPos.z - bias) / shadowPos.w) ));
  }

  return clamp(visibility, 0.0, 1.0);
}

float calcShadow2_2(
  in uint shadowIndex,
  in vec4 shadowPos,
  in vec3 normal,
  in vec3 toLight)
{
  // NOtE KI textureProj == automatic p.xyz / p.w

  if (shadowPos.z > 1.0) return 0.0;

  const float shadowDepth = shadowPos.z;

  float bias = max(0.03 * (1.0 - dot(normal, toLight)), 0.005);

  const vec2 texelSize = 1.0 / vec2(textureSize(u_shadowMap[shadowIndex], 0));

  float shadow = 0.0;

  for (int x = -1; x < 2; x++) {
    for (int y = -1; y < 2; y++) {
      vec4 pos = shadowPos + vec4(
        x * texelSize.x,
        y * texelSize.y,
        0.0,
        0.0);
      float pcf = textureProj(u_shadowMap[shadowIndex], pos);

      // Shadow acne correction
      shadow += shadowDepth - bias > pcf ? 0.0 : 0.5;
    }
  }
  return shadow / 9.0;
}

/*
float calcShadow3(
  in uint shadowIndex,
  in vec4 shadowPos,
  in vec3 normal,
  in vec3 toLight)
{
  vec3 projCoords = shadowPos.xyz / shadowPos.w;
  projCoords = projCoords * 0.5 + 0.5;
  float currentDepth = projCoords.z;

  // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
  if (currentDepth > 1.0) {
    return 1.0;
  }

  // calculate bias (based on depth map resolution and slope)
  float bias = max(0.05 * (1.0 - dot(normal, toLight)), 0.005);
  const float biasModifier = 0.5f;

  bias *= 1 / (u_shadowPlanes[shadowIndex + 1].z * biasModifier);

  // PCF
  float shadow = 0.0;

  vec2 texelSize = 1.0 / vec2(textureSize(u_shadowMap[shadowIndex], 0));

  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      // float pcfDepth = texture(u_shadowMap[shadowIndex], vec3(projCoords.xy + vec2(x, y) * texelSize, 1)).r;

      float pcfDepth =
        texture(
                u_shadowMap[shadowIndex],
                projCoords.xy + vec2(x, y) * texelSize).r;

      shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
    }
  }
  shadow /= 9.0;

  return 1.0 - shadow;
}
*/

vec4 calculateDirLight(
  in DirLight light,
  in vec3 normal,
  in vec3 toView,
  in vec3 worldPos,
  in uint shadowIndex,
  in vec4 shadowPos,
  in Material material)
{
  const vec3 toLight = normalize(-light.worldDir);

  // diffuse
  float diff = max(dot(normal, toLight), 0.0);
  vec4 diffuse = diff * light.diffuse * material.diffuse;

  // specular
  vec4 specular = vec4(0);
  if (material.shininess > 0) {
    vec3 reflectDir = reflect(-toLight, normal);
    float spec = pow(max(dot(toView, reflectDir), 0.0), material.shininess);
    specular = spec * light.specular * material.specular;
  }

  // calculate shadow
  float shadow = calcShadow2_3(worldPos, shadowIndex, shadowPos, normal, toLight);

  return shadow * (diffuse + specular);
}
