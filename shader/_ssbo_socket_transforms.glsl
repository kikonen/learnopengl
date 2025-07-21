#include struct_socket_transform.glsl

#define _SSBO_SOCKET_TRANSFORMS
layout (std430, binding = SSBO_SOCKET_TRANSFORMS) readonly buffer SocketTransformSSBO {
  SocketTransform u_socketTransforms[];
};

mat4 resolveSocketMatrix(uint index)
{
  const vec4 VEC_W = vec4(0, 0, 0, 1);

  return transpose(
    mat4(
      u_socketTransforms[index].u_transformRow0,
      u_socketTransforms[index].u_transformRow1,
      u_socketTransforms[index].u_transformRow2,
      VEC_W));
}
