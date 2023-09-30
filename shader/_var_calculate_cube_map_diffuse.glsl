#ifdef USE_CUBE_MAP
{
  float diffuseRatio = 1.0 - material.reflection - material.refraction;
  if (u_cubeMapExist && diffuseRatio < 1.0) {
    vec3 diffuse = material.diffuse.rgb * diffuseRatio;

    if (material.reflection > 0) {
      vec3 r = reflect(-viewDir, normal);
      diffuse += textureLod(u_cubeMap, r, 0).rgb * material.reflection;
    }

    if (material.refraction > 0) {
      vec3 r = refract(-viewDir, normal, material.refractionRatio);
      diffuse += textureLod(u_cubeMap, r, 0).rgb * material.refraction;
    }

    material.diffuse = vec4(diffuse, material.diffuse.a);
  }
}
#endif
