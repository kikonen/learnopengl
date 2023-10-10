#version 460 core

layout(vertices=3) out;

#include struct_entity.glsl
#include struct_clip_plane.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_clip_planes.glsl

in VS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;
  flat float tilingX;
  flat uvec2 heightMapTex;

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

  flat uint materialIndex;
  flat uvec2 heightMapTex;

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

void main()
{
  entity = u_entities[tcs_in[gl_InvocationID].entityIndex];
  #include var_entity_model_matrix.glsl

  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

  tcs_out[gl_InvocationID].entityIndex = tcs_in[gl_InvocationID].entityIndex;
  tcs_out[gl_InvocationID].worldPos = tcs_in[gl_InvocationID].worldPos;
  tcs_out[gl_InvocationID].normal = tcs_in[gl_InvocationID].normal;
  tcs_out[gl_InvocationID].texCoord = tcs_in[gl_InvocationID].texCoord;
  tcs_out[gl_InvocationID].vertexPos = tcs_in[gl_InvocationID].vertexPos;
  tcs_out[gl_InvocationID].materialIndex = tcs_in[gl_InvocationID].materialIndex;
  tcs_out[gl_InvocationID].heightMapTex = tcs_in[gl_InvocationID].heightMapTex;

#ifdef USE_TBN
  tcs_out[gl_InvocationID].tangent = tcs_in[gl_InvocationID].tangent;
#endif

  if (gl_InvocationID == 0) {
    // NOTE ratio scaling to retain more-or-less consistent level
    // with tile size changes
    const float ratio = 8.0 / tcs_in[gl_InvocationID].tilingX;

    const int MIN_TESS_LEVEL = 4;
    const int MAX_TESS_LEVEL = int(32 * ratio);
    const float MIN_DIST = 20;
    const float MAX_DIST = 700;
    const float DIST_DIFF = MAX_DIST - MIN_DIST;

    // NOTE KI use *world* not *view* space for distance
    // => to avoid terrain bumping around when rotating in place
    //mat4 mvMatrix = u_viewMatrix * modelMatrix;
    mat4 mvMatrix = modelMatrix;
    vec4 viewPos = vec4(u_viewWorldPos, 1.0);

    vec4 viewPos00 = viewPos - mvMatrix * gl_in[0].gl_Position;
    vec4 viewPos01 = viewPos - mvMatrix * gl_in[1].gl_Position;
    vec4 viewPos10 = viewPos - mvMatrix * gl_in[2].gl_Position;
    //vec4 viewPos11 = viewPos - mvMatrix * gl_in[3].gl_Position;

    // "distance" from camera scaled between 0 and 1
    float dist00 = clamp( (abs(viewPos00.z) - MIN_DIST) / (DIST_DIFF), 0.0, 1.0 );
    float dist01 = clamp( (abs(viewPos01.z) - MIN_DIST) / (DIST_DIFF), 0.0, 1.0 );
    float dist10 = clamp( (abs(viewPos10.z) - MIN_DIST) / (DIST_DIFF), 0.0, 1.0 );
    //float dist11 = clamp( (abs(viewPos11.z) - MIN_DIST) / (DIST_DIFF), 0.0, 1.0 );

    float tessLevel0 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist10, dist00) );
    float tessLevel1 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist00, dist01) );
    float tessLevel2 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist01, dist10) );
    //float tessLevel3 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist00, dist10) );

    float outerTessLevel = int(32 * ratio);
    gl_TessLevelOuter[0] = outerTessLevel; //tessLevel0;
    gl_TessLevelOuter[1] = outerTessLevel; //tessLevel1;
    gl_TessLevelOuter[2] = outerTessLevel; //tessLevel2;
    gl_TessLevelOuter[3] = outerTessLevel; //tessLevel3;

    gl_TessLevelInner[0] = max(tessLevel1, tessLevel2);
    gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
  }
}
