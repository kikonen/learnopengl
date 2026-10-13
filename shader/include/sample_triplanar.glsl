// references:
// - https://iquilezles.org/articles/biplanar/
// - https://catlikecoding.com/unity/tutorials/advanced-rendering/triplanar-mapping/
// - https://www.ronja-tutorials.com/post/010-triplanar-mapping/
vec3 sampleTriPlanar(vec3 pos, uint matIdx) {
  vec3 normal = normalize(cross(dFdx(pos), dFdy(pos)));

  float scale = 1.0;
  vec2 uvX = pos.zy * scale;
  vec2 uvY = pos.xz * scale;
  vec2 uvZ = pos.xy * scale;

  // Sharpen the blend so vertical walls cleanly favor X/Z projections instead
  // of muddy averages with the down-projected XZ stripe.
  vec3 absN = pow(abs(normal), vec3(4.0));
  vec3 blend = absN / (absN.x + absN.y + absN.z + 1e-5);

  const uint diffuseLayer = u_materials[matIdx].diffuseTex;

  vec3 cX = texture(u_texturesSRGB, vec3(uvX, float(diffuseLayer))).rgb;
  vec3 cY = texture(u_texturesSRGB, vec3(uvY, float(diffuseLayer))).rgb;
  vec3 cZ = texture(u_texturesSRGB, vec3(uvZ, float(diffuseLayer))).rgb;
  return cX * blend.x + cY * blend.y + cZ * blend.z;
}
