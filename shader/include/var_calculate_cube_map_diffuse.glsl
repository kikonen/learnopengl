#ifdef USE_CUBE_MAP
#define MAT_REFRACT_RATIO u_materials[materialIndex].refractionRatio
#define MAT_REFLECT u_materials[materialIndex].reflection
#define MAT_REFRACT u_materials[materialIndex].refraction

if (u_cubeMapEnabled) {
  float diffuseRatio = 1.0 - MAT_REFLECT - MAT_REFRACT;
  if (diffuseRatio < 1.0) {
    vec3 diffuse = material.diffuse.rgb * diffuseRatio;

    const mat3 invViewMat3 = mat3(u_invViewMatrix);

    if (MAT_REFLECT > 0) {
      // NOTE KI worldSpace coords needed
      const vec3 r = normalize(invViewMat3 * reflect(-viewDir, normal));

      diffuse += textureLod(u_cubeMap, r, 0).rgb * MAT_REFLECT;
    }

    if (MAT_REFRACT > 0) {
      // NOTE KI worldSpace coords needed
      const vec3 r = normalize(invViewMat3 * refract(-viewDir, normal, MAT_REFRACT_RATIO));

      diffuse += textureLod(u_cubeMap, r, 0).rgb * MAT_REFRACT;
    }

    material.diffuse = vec4(diffuse, material.diffuse.a);
  }
}
#endif
