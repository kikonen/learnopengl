layout (std430, binding = SSBO_BONE_TRANSFORMS) readonly buffer BoneTransformSSBO {
  BoneTransform u_boneTransforms[];
};
