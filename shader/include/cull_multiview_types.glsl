#define MAX_VIEWS 32          // Main + shadows + reflections
#define MAX_MESHES 64
#define MAX_CASCADES 4

// View types for specialized handling
#define VIEW_TYPE_CAMERA      0
#define VIEW_TYPE_DIRECTIONAL 1  // Orthographic shadow
#define VIEW_TYPE_SPOT        2  // Perspective shadow
#define VIEW_TYPE_POINT_FACE  3  // Cubemap face
#define VIEW_TYPE_REFLECTION  4  // Planar reflection

struct ViewData {
  mat4 viewProjection;
  vec4 frustumPlanes[6];
  vec4 position;           // xyz = position, w = unused
  vec4 direction;          // xyz = forward, w = unused
  uint viewType;
  uint viewIndex;          // index in output buffer array
  uint cascadeIndex;       // for CSM
  uint parentViewIndex;    // main camera for shadows (for LOD inheritance)
  vec4 lodDistances;       // per-view LOD thresholds
};

struct CullUniforms {
  uint instanceCount;
  uint viewCount;
  uint meshCount;
  uint frameIndex;
  vec4 mainCameraPos;      // for LOD calculations
};

// Per-view output allocation
struct ViewOutputInfo {
  uint outputBufferOffset; // where this view's instances start
  uint maxInstances;       // capacity for this view
  uint drawCommandOffset;  // where draw commands for this view start
  uint _pad;
};

// Per-instance input data (read-only during culling)
struct InstanceData {
    vec4 boundingSphere;    // xyz = center (world space), w = radius
    vec4 transform0;        // mat4 column 0 (or use quaternion + position)
    vec4 transform1;        // mat4 column 1
    vec4 transform2;        // mat4 column 2
    vec4 transform3;        // mat4 column 3 (position in xyz, scale in w if uniform)
    uint meshId;            // which mesh this instance uses
    uint flags;             // NO_CULL, ALWAYS_VISIBLE, etc.
    uint _pad0;
    uint _pad1;
};  // 96 bytes, aligned

// Compacted visible instance (output)
struct VisibleInstance {
    vec4 transform0;
    vec4 transform1;
    vec4 transform2;
    vec4 transform3;
};  // 64 bytes - only what vertex shader needs

// Per-mesh metadata
struct MeshInfo {
    uint indexCount;        // number of indices
    uint firstIndex;        // offset into index buffer
    int  baseVertex;        // added to each index
    uint maxInstances;      // capacity in output buffer (for bounds checking)
    uint outputOffset;      // where this mesh's instances start in VisibleInstances buffer
};

// Standard indirect draw command
struct DrawElementsIndirectCommand {
    uint count;             // index count
    uint instanceCount;     // filled by compute shader
    uint firstIndex;
    int  baseVertex;
    uint baseInstance;      // offset into VisibleInstances
};
