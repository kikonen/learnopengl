#define _SSBO_SOCKET_TRANSFORMS
layout (std430, binding = SSBO_SOCKET_TRANSFORMS) readonly buffer SocketTransformSSBO {
  mat4 u_socketTransforms[];
};
