{
  float diffuseRatio = 1.0 - material.reflection - material.refraction;
  if (u_cubeMapExist && diffuseRatio < 1.0) {
    vec3 diffuse = material.diffuse.rgb * diffuseRatio;

    if (material.reflection > 0) {
      vec3 r = reflect(-toView, normal);
      diffuse += texture(u_cubeMap, r).rgb * material.reflection;
    }

    if (material.refraction > 0) {
      vec3 r = refract(-toView, normal, material.refractionRatio);
      diffuse += texture(u_cubeMap, r).rgb * material.refraction;
    }

    material.diffuse = vec4(diffuse, material.diffuse.a);
  }
}
