#include struct_mesh_instance.glsl

#define _SSBO_MESH_INSTANCES
layout (std430, binding = SSBO_MESH_INSTANCES) readonly buffer MeshInstances {
  MeshInstance u_meshInstances[];
};
