// NOTE KI it's assumed that there is scaling is uniform
// - if not then normal/tangent are *incorrect*
//
// IN
// - a_boneIndex
// - a_boneWeight
// - u_boneTransforms
// - entity
// - pos
// - normal
// - tangent
// OUT
// - pos
// - normal
// - tangent
#ifdef USE_BONES
// NOTE KI u_boneBaseIndex == 0 is NULL entry
// => this broke normal debug rendering
if (entity.u_boneBaseIndex > 0) {
  const uvec4 boneIndex = a_boneIndex + entity.u_boneBaseIndex;
  const vec4 boneWeight = a_boneWeight;

  const mat4 b1 = resolveBoneMatrix(boneIndex.x);
  const mat4 b2 = resolveBoneMatrix(boneIndex.y);
  const mat4 b3 = resolveBoneMatrix(boneIndex.z);
  const mat4 b4 = resolveBoneMatrix(boneIndex.w);

  // const mat4 b1 = mat4(1.f);
  // const mat4 b2 = mat4(1.f);
  // const mat4 b3 = mat4(1.f);
  // const mat4 b4 = mat4(1.f);

  const float wx = boneWeight.x;
  const float wy = boneWeight.y;
  const float wz = boneWeight.z;
  const float ww = boneWeight.w;

  const vec4 ZERO = vec4(0);

  // Skin the position
  vec4 skinPos =
      (wx > 0 ? (b1 * pos) * wx : ZERO)
    + (wy > 0 ? (b2 * pos) * wy : ZERO)
    + (wz > 0 ? (b3 * pos) * wz : ZERO)
    + (ww > 0 ? (b4 * pos) * ww : ZERO);

  // https://www.khronos.org/opengl/wiki/Skeletal_Animation
  pos = vec4(skinPos.xyz, 1.0);

#ifdef USE_BONES_NORMAL
  // Skin the normal
  // NOTE KI w = 0.0 for w for normal
  vec4 skinNormal = vec4(normal, 0.f);
  skinNormal =
      (wx > 0 ? (b1 * skinNormal) * wx : ZERO)
    + (wy > 0 ? (b2 * skinNormal) * wy : ZERO)
    + (wz > 0 ? (b3 * skinNormal) * wz : ZERO)
    + (ww > 0 ? (b4 * skinNormal) * ww : ZERO);
  normal = skinNormal.xyz;
#endif

#ifdef USE_TBN
  // Skin the tangent
  // NOTE KI w = 0.0 for w for tangent
  vec4 skinTangent = vec4(tangent, 0.f);
  skinTangent =
      (wx > 0 ? (b1 * skinTangent) * wx : ZERO)
    + (wy > 0 ? (b2 * skinTangent) * wy : ZERO)
    + (wz > 0 ? (b3 * skinTangent) * wz : ZERO)
    + (ww > 0 ? (b4 * skinTangent) * ww : ZERO);
  tangent = skinTangent.xyz;
#endif
}
#endif
