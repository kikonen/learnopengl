#version 460 core

layout(vertices=4) out;

#include struct_entity.glsl
#include struct_clip_plane.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

in VS_OUT {
  flat uint entityIndex;

  vec3 normal;
  vec2 texCoord;
  vec4 vertexPos;

  flat uint materialIndex;

  vec3 scale;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} ts_in[];

out TCS_OUT {
  flat uint entityIndex;

  vec3 normal;
  vec2 texCoord;
  vec4 vertexPos;

  flat uint materialIndex;

  vec3 scale;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} ts_out[];

//out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

//#include fn_calculate_clipping.glsl

void main()
{
  const Entity entity = u_entities[ts_in[gl_InvocationID].entityIndex];
  #include var_entity_model_matrix.glsl

  //calculateClipping(ts_in[gl_InvocationID].worldPos);

  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

  ts_out[gl_InvocationID].entityIndex = ts_in[gl_InvocationID].entityIndex;
  ts_out[gl_InvocationID].normal = ts_in[gl_InvocationID].normal;
  ts_out[gl_InvocationID].texCoord = ts_in[gl_InvocationID].texCoord;
  ts_out[gl_InvocationID].vertexPos = ts_in[gl_InvocationID].vertexPos;
  ts_out[gl_InvocationID].materialIndex = ts_in[gl_InvocationID].materialIndex;
  ts_out[gl_InvocationID].scale = ts_in[gl_InvocationID].scale;

#ifdef USE_NORMAL_TEX
  ts_out[gl_InvocationID].TBN = ts_in[gl_InvocationID].TBN;
#endif

  if (gl_InvocationID == 0) {
    const int MIN_TESS_LEVEL = 4;
    const int MAX_TESS_LEVEL = 64;
    const float MIN_DISTANCE = 20;
    const float MAX_DISTANCE = 800;

    mat4 mvMatrix = u_viewMatrix * modelMatrix;

    vec4 eyeSpacePos00 = mvMatrix * gl_in[0].gl_Position;
    vec4 eyeSpacePos01 = mvMatrix * gl_in[1].gl_Position;
    vec4 eyeSpacePos10 = mvMatrix * gl_in[2].gl_Position;
    vec4 eyeSpacePos11 = mvMatrix * gl_in[3].gl_Position;

    // "distance" from camera scaled between 0 and 1
    float distance00 = clamp( (abs(eyeSpacePos00.z) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0 );
    float distance01 = clamp( (abs(eyeSpacePos01.z) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0 );
    float distance10 = clamp( (abs(eyeSpacePos10.z) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0 );
    float distance11 = clamp( (abs(eyeSpacePos11.z) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0 );

    float tessLevel0 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance10, distance00) );
    float tessLevel1 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance00, distance01) );
    float tessLevel2 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance01, distance11) );
    float tessLevel3 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance11, distance10) );

    gl_TessLevelOuter[0] = tessLevel0;
    gl_TessLevelOuter[1] = tessLevel1;
    gl_TessLevelOuter[2] = tessLevel2;
    gl_TessLevelOuter[3] = tessLevel3;

    gl_TessLevelInner[0] = max(tessLevel1, tessLevel3);
    gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
  }
}
