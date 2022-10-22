// NOTE KI *Too* big (like 32) array *will* cause shader to crash mysteriously
// => defined by c++ side (more optimal)
#ifndef MAT_COUNT
  #define MAT_COUNT 8
#endif

// NOTE KI TEX_COUNT == texture index != texture unit index
// => i.e. unit index is not 1:1 mapping to texture index
#ifndef TEX_COUNT
  #define TEX_COUNT 8
#endif

layout (std140) uniform Materials {
  Material materials[MAT_COUNT];
};
