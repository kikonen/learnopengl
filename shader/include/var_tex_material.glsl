// #define materialRGBA(tx) \
//   if (material.tx ## Tex.x > 0) { \
//     material.tx = texture(sampler2D(material.tx ## Tex), texCoord); \
//   }
#ifndef USE_TEXTURE_ARRAY
{
  const uint i = materialIndex;

  material.flags = u_materials[i].flags;

  vec4 mrasTex = vec4(0, 1, 1, 0);

  if (u_materials[i].mrasMapTex.x > 0) {
    mrasTex = texture(sampler2D(u_materials[i].mrasMapTex), texCoord).rgba;

    if ((material.flags & MATERIAL_INVERT_METALNESS) != 0)
    {
      MRA_TEX_METALNESS = 1.0 - MRA_TEX_METALNESS;
    }
    if ((material.flags & MATERIAL_INVERT_ROUGHNESS) != 0)
    {
      MRA_TEX_ROUGHNESS = 1.0 - MRA_TEX_ROUGHNESS;
    }
    if ((material.flags & MATERIAL_INVERT_OCCLUSION) != 0)
    {
      MRA_TEX_OCCLUSION = 1.0 - MRA_TEX_OCCLUSION;
    }
  }

  vec4 mras = u_materials[i].mras.rgba * mrasTex.rgba;

#ifndef _ALPHA_RESOLVED
  material.diffuseTexel = texture(sampler2D(u_materials[i].diffuseTex), texCoord);

#ifdef USE_ALPHA
  material.alpha =
    (u_materials[materialIndex].diffuse.a *
     material.diffuseTexel.a *
    texture(sampler2D(u_materials[materialIndex].opacityMapTex), texCoord).r);
#else
  material.alpha = 1.0;
#endif
#endif

  material.diffuse = u_materials[i].diffuse * material.diffuseTexel;
  material.diffuse.a = material.alpha;

  // NOTE KI discard any trash, which is possibly hidden into emission tex with alpha
  // thus (0, 0, 0) == (r, g, b, 0)
  vec4 emission = texture(
    sampler2D(u_materials[i].emissionTex),
    texCoord + vec2(0, u_time) * -0);

  material.emission = u_materials[i].emission.rgb *
    emission.rgb * emission.a;

  material.mras = mras.rgba;

  // material.reflection = u_materials[i].reflection;
  // material.refraction = u_materials[i].refraction;
  // material.refractionRatio = u_materials[i].refractionRatio;
}
#endif

#ifdef USE_TEXTURE_ARRAY
{
  const uint i = materialIndex;

  material.flags = u_materials[i].flags;

  // ==========================================
  // PBR MRAS TEXTURE RESOLUTION (Linear Pool)
  // ==========================================

  // Default placeholder: Roughness=1.0, Metallic=1.0, AO=1.0
  vec4 mrasTex = vec4(0.0, 1.0, 1.0, 0.0);

  int mrasLayer = int(u_materials[i].mrasMapTex.x);

  // Isolate sampling logic to valid layers only
  if (mrasLayer >= 0) {
    mrasTex = texture(u_TexturesLinear, vec3(texCoord, float(mrasLayer))).rgba;

    // Apply metallic channel data inversion mapping
    if ((material.flags & MATERIAL_INVERT_METALNESS) != 0) {
      mrasTex.g = 1.0 - mrasTex.g;
    }
    // Apply roughness channel data inversion mapping
    if ((material.flags & MATERIAL_INVERT_ROUGHNESS) != 0) {
      mrasTex.r = 1.0 - mrasTex.r;
    }
    // Apply occlusion channel data inversion mapping
    if ((material.flags & MATERIAL_INVERT_OCCLUSION) != 0) {
      mrasTex.b = 1.0 - mrasTex.b;
    }
  }

  // Cross-multiply factors across resolved layout values
  vec4 mras = u_materials[i].mras.rgba * mrasTex.rgba;
  material.mras = mras.rgba;

  // ==========================================
  // DIFFUSE & OPACITY RESOLUTION (sRGB Pool)
  // ==========================================

  int diffuseLayer = int(u_materials[i].diffuseTex.x);

  // Sample unified sRGB textures array
  material.diffuseTexel = texture(u_TexturesSRGB, vec3(texCoord, float(diffuseLayer)));

#ifndef _ALPHA_RESOLVED
  #ifdef USE_ALPHA
    // Evaluate alpha utilizing baked diffuse texture component data
    material.alpha = u_materials[materialIndex].diffuse.a * material.diffuseTexel.a;
  #else
    // Force absolute opaque state for regular solid drawing paths
    material.alpha = 1.0;
  #endif
#endif

  // Apply final color scaling matrix operations
  material.diffuse = u_materials[i].diffuse * material.diffuseTexel;
  material.diffuse.a = material.alpha;

  // ==========================================
  // 3. EMISSION / LUMINANCE RESOLUTION (sRGB Pool)
  // ==========================================
  int emissionLayer = int(u_materials[i].emissionTex.x);
  vec4 emission = vec4(0.0);

  if (emissionLayer >= 0) {
    // Read the emissive color maps from the same hardware-linearizing sRGB container pool
    emission = texture(u_TexturesSRGB, vec3(texCoord + vec2(0.0, u_time) * -0.0, float(emissionLayer)));
  }

  // Safely strip away any unintentional color leaks stored in the emissive alpha layer channel
  material.emission = u_materials[i].emission.rgb * emission.rgb * emission.a;
}
#endif
