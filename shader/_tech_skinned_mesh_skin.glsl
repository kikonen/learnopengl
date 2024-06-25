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
{
  const uvec4 boneIndex = a_boneIndex + entity.u_boneBaseIndex;
  const vec4 boneWeight = a_boneWeight;

  const mat4 b1 = u_boneTransforms[boneIndex.x];
  const mat4 b2 = u_boneTransforms[boneIndex.y];
  const mat4 b3 = u_boneTransforms[boneIndex.z];
  const mat4 b4 = u_boneTransforms[boneIndex.w];

  // const mat4 b1 = mat4(1.f);
  // const mat4 b2 = mat4(1.f);
  // const mat4 b3 = mat4(1.f);
  // const mat4 b4 = mat4(1.f);

  // Skin the position
  const vec4 skinPos = (b1 * pos) * boneWeight.x
    + (b2 * pos) * boneWeight.y
    + (b3 * pos) * boneWeight.z
    + (b4 * pos) * boneWeight.w;
  pos = skinPos;

#ifdef USE_BONES_NORMAL
  // Skin the normal
  // NOTE KI w = 0.0 for w for normal
  vec4 skinNormal = vec4(normal, 0.f);
  skinNormal = (b1 * skinNormal) * boneWeight.x
    + (b2 * skinNormal) * boneWeight.y
    + (b3 * skinNormal) * boneWeight.z
    + (b4 * skinNormal) * boneWeight.w;
  normal = skinNormal.xyz;
#endif

#ifdef USE_TBN
  // Skin the tangent
  // NOTE KI w = 0.0 for w for tangent
  vec4 skinTangent = vec4(tangent, 0.f);
  skinTangent = (b1 * skinTangent) * boneWeight.x
    + (b2 * skinTangent) * boneWeight.y
    + (b3 * skinTangent) * boneWeight.z
    + (b4 * skinTangent) * boneWeight.w;
  tangent = skinTangent.xyz;
#endif
}
#endif
