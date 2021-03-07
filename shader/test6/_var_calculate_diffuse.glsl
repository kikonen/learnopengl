{
  float diffuseRatio = 1.0 - material.reflection - material.refraction;
  if (diffuseRatio < 1.0) {
    vec3 diffuse = material.diffuse.rgb * diffuseRatio;

    if (material.reflection > 0) {
      vec3 r = reflect(-toView, normal);
      diffuse += texture(reflectionMap, r).rgb * material.reflection;
    }

    if (material.refraction > 0) {
      float ratio = 1.0 / 1.33;
      vec3 r = refract(-toView, normal, ratio);
      diffuse += texture(refractionMap, r).rgb * material.refraction;
    }

    material.diffuse = vec4(diffuse, material.diffuse.a);
  }
}
