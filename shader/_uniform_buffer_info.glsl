#define _UBO_BUFFER_INFO
layout(std140, binding = UBO_BUFFER_INFO) uniform BufferInfo {
  vec2 u_bufferResolution;
};
