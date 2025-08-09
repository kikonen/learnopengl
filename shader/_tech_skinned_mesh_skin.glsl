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
#define W1 a_boneWeight.x
#define W2 a_boneWeight.y
#define W3 a_boneWeight.z
#define W4 a_boneWeight.w

// NOTE KI u_boneBaseIndex == 0 is NULL entry
// => this broke normal debug rendering
if (entity.u_boneBaseIndex > 0) {
  const uvec4 boneIndex = a_boneIndex + entity.u_boneBaseIndex;

  // Skin the position
  const mat4 skinMat =
    W1 * resolveBoneMatrix(boneIndex.x) +
    W2 * resolveBoneMatrix(boneIndex.y) +
    W3 * resolveBoneMatrix(boneIndex.z) +
    W4 * resolveBoneMatrix(boneIndex.w);

  vec4 skinPos = skinMat * pos;

  // https://www.khronos.org/opengl/wiki/Skeletal_Animation
  pos = vec4(skinPos.xyz, 1.0);

#ifdef USE_BONES_NORMAL
  // NOTE KI w = 0.0 for w for normal
  normal = normalize(vec3(skinMat * vec4(normal, 0.0)));
#endif

#ifdef USE_TBN
  // NOTE KI w = 0.0 for w for tangent
  tangent = normalize(vec3(skinMat * vec4(tangent, 0.0)));
#endif
}

#undef W1
#undef W2
#undef W3
#undef W4
#endif
