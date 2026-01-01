#ifdef USE_BONES
layout (location = ATTR_BONE_INDEX) in uvec4 a_boneIndex;
layout (location = ATTR_BONE_WEIGHT) in vec4 a_boneWeight;

#include include/ssbo_bone_transforms.glsl
#endif
