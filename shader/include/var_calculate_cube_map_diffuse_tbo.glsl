#ifdef USE_CUBE_MAP

if (u_cubeMapEnabled) {
  float diffuseRatio = 1.0 - material.reflection - material.refraction;
  if (diffuseRatio < 1.0) {
    vec3 diffuse = material.diffuse.rgb * diffuseRatio;

    const mat3 invViewMat3 = mat3(u_invViewMatrix);

    if (material.reflection > 0) {
      // NOTE KI worldSpace coords needed
      const vec3 r = normalize(invViewMat3 * reflect(-viewDir, normal));

      diffuse += textureLod(u_cubeMap, r, 0).rgb * material.reflection;
    }

    if (material.refraction > 0) {
      // NOTE KI worldSpace coords needed
      const vec3 r = normalize(invViewMat3 * refract(-viewDir, normal, material.refractionRatio));

      diffuse += textureLod(u_cubeMap, r, 0).rgb * material.refraction;
    }

    material.diffuse = vec4(diffuse, material.diffuse.a);
  }
}
#endif
