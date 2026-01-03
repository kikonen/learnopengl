#version 460 core

layout(vertices=3) out;

#include "include/struct_material.glsl"
#include "include/struct_entity.glsl"
#include "include/struct_clip_plane.glsl"

#include "include/ssbo_entities.glsl"
#include "include/uniform_matrices.glsl"
#include "include/ssbo_materials.glsl"
#include "include/uniform_clip_planes.glsl"

in VS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  flat uint shadowIndex;
  vec4 shadowPos;

#ifdef USE_TBN
  vec3 tangent;
#endif
} tcs_in[];

out TCS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  flat uint shadowIndex;
  vec4 shadowPos;

#ifdef USE_TBN
  vec3 tangent;
#endif
} tcs_out[];

//out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Entity entity;

//#include "include/fn_calculate_clipping.glsl"

void main()
{
  entity = u_entities[tcs_in[gl_InvocationID].entityIndex];
  #include "include/var_entity_model_matrix.glsl"

  const uint materialIndex = instance.u_materialIndex;

  //calculateClipping(tcs_in[gl_InvocationID].worldPos);

  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

  tcs_out[gl_InvocationID].entityIndex = tcs_in[gl_InvocationID].entityIndex;
  tcs_out[gl_InvocationID].worldPos = tcs_in[gl_InvocationID].worldPos;
  tcs_out[gl_InvocationID].normal = tcs_in[gl_InvocationID].normal;
  tcs_out[gl_InvocationID].texCoord = tcs_in[gl_InvocationID].texCoord;
  tcs_out[gl_InvocationID].vertexPos = tcs_in[gl_InvocationID].vertexPos;
  tcs_out[gl_InvocationID].viewPos = tcs_in[gl_InvocationID].viewPos;
  tcs_out[gl_InvocationID].materialIndex = tcs_in[gl_InvocationID].materialIndex;
  tcs_out[gl_InvocationID].shadowIndex = tcs_in[gl_InvocationID].shadowIndex;
  tcs_out[gl_InvocationID].shadowPos = tcs_in[gl_InvocationID].shadowPos;

#ifdef USE_TBN
  tcs_out[gl_InvocationID].tangent = tcs_in[gl_InvocationID].tangent;
#endif

  if (gl_InvocationID == 0) {
    // NOTE ratio scaling to retain more-or-less consistent level
    // with tile size changes
    const float ratio = 8.0 / u_materials[materialIndex].tilingX;

    const int MIN_TESS_LEVEL = 4;
    const int MAX_TESS_LEVEL = int(32 * ratio);
    const float MIN_DIST = 20;
    const float MAX_DIST = 700;
    const float DIST_DIFF = MAX_DIST - MIN_DIST;

    mat4 mvMatrix = u_viewMatrix * modelMatrix;

    vec4 viewPos00 = mvMatrix * gl_in[0].gl_Position;
    vec4 viewPos01 = mvMatrix * gl_in[1].gl_Position;
    vec4 viewPos10 = mvMatrix * gl_in[2].gl_Position;
    //vec4 viewPos11 = mvMatrix * gl_in[3].gl_Position;

    // "distance" from camera scaled between 0 and 1
    float dist00 = clamp( (abs(viewPos00.z) - MIN_DIST) / (DIST_DIFF), 0.0, 1.0 );
    float dist01 = clamp( (abs(viewPos01.z) - MIN_DIST) / (DIST_DIFF), 0.0, 1.0 );
    float dist10 = clamp( (abs(viewPos10.z) - MIN_DIST) / (DIST_DIFF), 0.0, 1.0 );
    //float dist11 = clamp( (abs(viewPos11.z) - MIN_DIST) / (DIST_DIFF), 0.0, 1.0 );

    float tessLevel0 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist10, dist00) );
    float tessLevel1 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist00, dist01) );
    float tessLevel2 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist01, dist10) );
    //float tessLevel3 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist00, dist10) );

    float outerTessLevel = 8;
    gl_TessLevelOuter[0] = outerTessLevel; //tessLevel0;
    gl_TessLevelOuter[1] = outerTessLevel; //tessLevel1;
    gl_TessLevelOuter[2] = outerTessLevel; //tessLevel2;
    gl_TessLevelOuter[3] = outerTessLevel; //tessLevel3;

    gl_TessLevelInner[0] = max(tessLevel1, tessLevel2);
    gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
  }
}
