#define _SSBO_BONE_TRANSFORM
layout (std430, binding = SSBO_BONE_TRANSFORMS) readonly buffer BoneTransformSSBO {
  mat4 u_boneTransforms[];
};
