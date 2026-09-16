// #define materialRGBA(tx) \
//   if (material.tx ## Tex.x > 0) { \
//     material.tx = texture(sampler2D(material.tx ## Tex), texCoord); \
//   }

{
  const uint i = materialIndex;

  material.flags = u_materials[i].flags;

  // ==========================================
  // PBR MRAS TEXTURE RESOLUTION (Linear Pool)
  // ==========================================

  // Default placeholder: Roughness=1.0, Metallic=1.0, AO=1.0
  vec4 mrasTex = vec4(0.0, 1.0, 1.0, 0.0);

  const uint mrasLayer = u_materials[i].mrasMapTex;

  // Isolate sampling logic to valid layers only
  if (mrasLayer > 0) {
    mrasTex = texture(
      u_texturesLinear,
      vec3(texCoord, float(mrasLayer))).rgba;

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

  // Cross-multiply factors across resolved layout values
  vec4 mras = u_materials[i].mras.rgba * mrasTex.rgba;
  material.mras = mras.rgba;

  // ==========================================
  // DIFFUSE & OPACITY RESOLUTION (sRGB Pool)
  // ==========================================

#ifndef _ALPHA_RESOLVED
#ifdef USE_DYNAMIC_TEXTURE
  {
    const int diffuseLayer = int(u_materials[i].diffuseTex);
    const int dynamicLayer = int(readMaterial_dynamicTex(i));

    vec4 staticTexel  = texture(
      u_texturesSRGB,
      vec3(texCoord, float(diffuseLayer)));

    vec4 dynamicTexel = texture(
      u_texturesDynamic,
      vec3(texCoord, float(dynamicLayer)));

    float mixRatio = readMaterial_dynamicRatio(materialIndex);

    material.diffuseTexel = mix(staticTexel, dynamicTexel, mixRatio);
    material.diffuseTexel.rgb = mix(staticTexel.rgb, dynamicTexel.rgb, mixRatio);

#ifdef USE_ALPHA
#ifdef USE_BLEND
  material.diffuseTexel.a = mix(staticTexel.a, dynamicTexel.a, mixRatio);
  // material.diffuseTexel.a = min(staticTexel.a, dynamicTexel.a);
  // material.diffuseTexel.a = dynamicTexel.a;
#else
  material.diffuseTexel.a = staticTexel.a;
#endif
#endif
  }
#else
  {
    const int diffuseLayer = int(u_materials[i].diffuseTex);

    // Sample unified sRGB textures array
    material.diffuseTexel = texture(
      u_texturesSRGB,
      vec3(texCoord, float(diffuseLayer)));
  }
#endif

  #ifdef USE_ALPHA
    // Evaluate alpha utilizing baked diffuse texture component data
    material.alpha = u_materials[materialIndex].diffuse.a * material.diffuseTexel.a;
  #else
    // Force absolute opaque state for regular solid drawing paths
    material.diffuseTexel.a = 1.0;
    material.alpha = 1.0;
  #endif
#endif

  // Apply final color scaling matrix operations
  material.diffuse = u_materials[i].diffuse * material.diffuseTexel;
  material.diffuse.a = material.alpha;

  // ==========================================
  // 3. EMISSION / LUMINANCE RESOLUTION (sRGB Pool)
  // ==========================================
  const int emissionLayer = int(u_materials[i].emissionTex);

  vec4 emission = vec4(0.0);

  if (emissionLayer > 0) {
    // Read the emissive color maps from the same hardware-linearizing sRGB container pool
    emission = texture(
      u_texturesSRGB,
      vec3(texCoord + vec2(0.0, u_time) * -0.0, float(emissionLayer)));
  }

  // Safely strip away any unintentional color leaks stored in the emissive alpha layer channel
  material.emission = u_materials[i].emission.rgb * emission.rgb * emission.a;
}
