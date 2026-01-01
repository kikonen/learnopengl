#include include/struct_bone_transform.glsl

#define _SSBO_BONE_TRANSFORM
layout (std430, binding = SSBO_BONE_TRANSFORMS) readonly buffer BoneTransformSSBO {
  BoneTransform u_boneTransforms[];
};

mat4 resolveBoneMatrix(uint index)
{
  const vec4 VEC_W = vec4(0, 0, 0, 1);

  return transpose(
    mat4(
      u_boneTransforms[index].u_transformRow0,
      u_boneTransforms[index].u_transformRow1,
      u_boneTransforms[index].u_transformRow2,
      VEC_W));
}
