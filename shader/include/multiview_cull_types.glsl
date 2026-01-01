// ============================================================
// multiview_cull_types.glsl
// ============================================================

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
