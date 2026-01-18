#version 460 core

layout(local_size_x = 256) in;

#include "include/cull_multiview_types.glsl"

// ------------------------------------------------------------
// Buffers
// ------------------------------------------------------------

layout(std430, binding = SSBO_CULL_INPUT_INSTANCES) readonly buffer InputInstancesSSBO {
  InstanceData u_instances[];
};

// Single large output buffer, partitioned per-view
layout(std430, binding = SSBO_CULL_OUTPUT_INSTANCES) writeonly buffer OutputInstancesSSBO {
  VisibleInstance u_visible[];
};

layout(std430, binding = SSBO_CULL_MESH_INFOS) readonly buffer MeshInfosSSBO {
  MeshInfo u_meshes[];
};

layout(std430, binding = SSBO_CULL_VIEWS) readonly buffer ViewsSSBO {
  ViewData u_views[];
};

layout(std430, binding = SSBO_CULL_VIEW_OUTPUT_INFOS) readonly buffer ViewOutputInfosSSBO {
  ViewOutputInfo u_viewOutputs[];
};

// Atomic counters: [viewIndex * MAX_MESHES + meshId]
layout(std430, binding = SSBO_CULL_VISIBLE_COUNTS) buffer VisibleCountsSSBO {
  uint u_visibleCounts[];
};

layout(std140, binding = UBO_CULL_UNIFORMS) uniform CullUniformsBlock {
  CullUniforms u_cull;
};

// ------------------------------------------------------------
// Shared memory
// ------------------------------------------------------------

// Visibility bitmask per thread (which views can see this instance)
// Using uvec2 for up to 64 views, or uint for up to 32
shared uint s_visibilityMask[256];
shared uint s_meshIds[256];
shared uint s_lodLevels[256];

// Per-view, per-mesh local counts (for workgroup accumulation)
// This gets large - may need to limit views processed per dispatch
#define VIEWS_PER_BATCH 8
shared uint s_localCounts[VIEWS_PER_BATCH][MAX_MESHES];
shared uint s_localBases[VIEWS_PER_BATCH][MAX_MESHES];

// ------------------------------------------------------------
// Culling functions
// ------------------------------------------------------------

bool sphereInFrustum(vec4 planes[6], vec3 center, float radius) {
  for (int i = 0; i < 6; i++) {
    if (dot(planes[i].xyz, center) + planes[i].w < -radius) {
      return false;
    }
  }
  return true;
}

// Optimized sphere-vs-ortho-box for directional shadows
bool sphereInOrthoFrustum(ViewData view, vec3 center, float radius) {
  // For orthographic, we can use simpler AABB-style test
  // The frustum planes still work, but this can be faster
  return sphereInFrustum(view.frustumPlanes, center, radius);
}

// For point lights: sphere vs sphere (light radius)
bool sphereInPointLightRange(ViewData view, vec3 center, float radius) {
  float dist = distance(view.position.xyz, center);
  float lightRadius = view.lodDistances.w;  // repurpose for light range
  return dist - radius < lightRadius;
}

bool isVisibleToView(ViewData view, vec3 center, float radius) {
  switch (view.viewType) {
    case VIEW_TYPE_CAMERA:
    case VIEW_TYPE_SPOT:
    case VIEW_TYPE_REFLECTION:
      return sphereInFrustum(view.frustumPlanes, center, radius);

    case VIEW_TYPE_DIRECTIONAL:
      return sphereInOrthoFrustum(view, center, radius);

    case VIEW_TYPE_POINT_FACE:
      // Could do per-face culling or just range check
      return sphereInPointLightRange(view, center, radius);

    default:
      return true;
  }
}

uint selectLOD(vec3 instancePos, float radius, vec4 lodDistances, vec3 cameraPos) {
  float dist = distance(instancePos, cameraPos) - radius;
  if (dist < lodDistances.x) return 0;
  if (dist < lodDistances.y) return 1;
  if (dist < lodDistances.z) return 2;
  return 3;
}

uint getMeshIdForLOD(InstanceData instance, uint lod) {
  // Assuming LOD mesh IDs packed in instance
  switch (lod) {
    case 0: return instance.meshLOD0;
    case 1: return instance.meshLOD1;
    case 2: return instance.meshLOD2;
    default: return instance.meshLOD3;
  }
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

void main() {
  uint globalIdx = gl_GlobalInvocationID.x;
  uint localIdx = gl_LocalInvocationIndex;

  // We process views in batches to limit shared memory usage
  uint viewBatchCount = (u_cull.viewCount + VIEWS_PER_BATCH - 1) / VIEWS_PER_BATCH;

  // Load instance data once
  InstanceData instance;
  vec3 center;
  float radius;
  bool validInstance = globalIdx < u_cull.instanceCount;

  if (validInstance) {
    instance = u_instances[globalIdx];
    center = instance.boundingSphere.xyz;
    radius = instance.boundingSphere.w;
  }

  // Process each batch of views
  for (uint viewBatch = 0; viewBatch < viewBatchCount; viewBatch++) {
    uint viewStart = viewBatch * VIEWS_PER_BATCH;
    uint viewEnd = min(viewStart + VIEWS_PER_BATCH, u_cull.viewCount);
    uint viewsThisBatch = viewEnd - viewStart;

    // ----------------------------------------------------
    // Phase 1: Initialize shared memory for this batch
    // ----------------------------------------------------
    for (uint v = localIdx; v < viewsThisBatch * MAX_MESHES; v += 256) {
      uint viewLocal = v / MAX_MESHES;
      uint meshId = v % MAX_MESHES;
      s_localCounts[viewLocal][meshId] = 0;
    }
    s_visibilityMask[localIdx] = 0;

    barrier();

    // ----------------------------------------------------
    // Phase 2: Test visibility against all views in batch
    // ----------------------------------------------------
    uint visMask = 0;
    uint lodLevel = 0;
    uint meshId = 0;

    if (validInstance && (instance.flags & FLAG_DISABLED) == 0) {
      bool noCull = (instance.flags & FLAG_NO_CULL) != 0;

      // LOD from main camera (view 0), shared across shadow views
      lodLevel = selectLOD(center,
                           radius,
                           u_views[0].lodDistances,
                           u_cull.mainCameraPos.xyz);
      meshId = getMeshIdForLOD(instance, lodLevel);

      for (uint v = 0; v < viewsThisBatch; v++) {
        uint viewIdx = viewStart + v;
        ViewData view = u_views[viewIdx];

        bool visible = noCull || isVisibleToView(view, center, radius);

        if (visible) {
          visMask |= (1u << v);
        }
      }
    }

    s_visibilityMask[localIdx] = visMask;
    s_meshIds[localIdx] = meshId;
    s_lodLevels[localIdx] = lodLevel;

    barrier();

    // ----------------------------------------------------
    // Phase 3: Count visible instances per view per mesh
    // ----------------------------------------------------
    if (validInstance && visMask != 0) {
      for (uint v = 0; v < viewsThisBatch; v++) {
        if ((visMask & (1u << v)) != 0) {
          atomicAdd(s_localCounts[v][meshId], 1);
        }
      }
    }

    barrier();

    // ----------------------------------------------------
    // Phase 4: Allocate global slots
    // ----------------------------------------------------
    // One thread per (view, mesh) pair does global atomic
    for (uint v = localIdx; v < viewsThisBatch * u_cull.meshCount; v += 256) {
      uint viewLocal = v / u_cull.meshCount;
      uint m = v % u_cull.meshCount;
      uint viewIdx = viewStart + viewLocal;

      uint localCount = s_localCounts[viewLocal][m];
      if (localCount > 0) {
        uint counterIdx = viewIdx * MAX_MESHES + m;
        s_localBases[viewLocal][m] = atomicAdd(u_visibleCounts[counterIdx], localCount);
      }
    }

    barrier();

    // ----------------------------------------------------
    // Phase 5: Write to per-view output buffers
    // ----------------------------------------------------
    if (validInstance && visMask != 0) {
      // Need per-thread slot tracking within each view
      // This requires a second local atomic pass - simplified here

      for (uint v = 0; v < viewsThisBatch; v++) {
        if ((visMask & (1u << v)) != 0) {
          uint viewIdx = viewStart + v;
          ViewOutputInfo outInfo = u_viewOutputs[viewIdx];
          MeshInfo mesh = u_meshes[meshId];

          // Get slot (simplified - real impl needs per-thread tracking)
          uint counterIdx = viewIdx * MAX_MESHES + meshId;

          // Would need separate counter
          uint slot = atomicAdd(u_visibleCounts[counterIdx], 0);

          // This is simplified - proper implementation needs
          // workgroup-local slot assignment like in single-view version
          uint outputIdx = outInfo.outputBufferOffset + mesh.outputOffset + slot;

          // Write instance
          u_visible[outputIdx] = VisibleInstance(
            instance.transform0,
            instance.transform1,
            instance.transform2,
            instance.transform3
          );
        }
      }
    }

    barrier();
  }
}
