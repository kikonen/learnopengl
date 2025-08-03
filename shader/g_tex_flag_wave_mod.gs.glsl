#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

#include uniform_data.glsl
#include uniform_clip_planes.glsl

in gl_PerVertex
{
  vec4 gl_Position;
#ifdef USE_GL_POINTS
  float gl_PointSize;
#endif
  float gl_ClipDistance[CLIP_COUNT];
} gl_in[3];

in VS_OUT {
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 objectPos;
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
  flat uint flags;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif

#ifdef USE_BONES
#ifdef USE_DEBUG
  flat uint boneBaseIndex;
  flat uvec4 boneIndex;
  flat vec4 boneWeight;
  vec3 boneColor;
#endif
#endif
#ifdef USE_DEBUG
  flat uint socketIndex;
#endif
} gs_in[];

out VS_OUT {
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 objectPos;
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
  flat uint flags;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif

#ifdef USE_BONES
#ifdef USE_DEBUG
  flat uint boneBaseIndex;
  flat uvec4 boneIndex;
  flat vec4 boneWeight;
  vec3 boneColor;
#endif
#endif
#ifdef USE_DEBUG
  flat uint socketIndex;
#endif
} gs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;


void sendVertex(in int i, in vec4 pos) {
  vec3 op = gs_in[i].objectPos;
  gl_Position = pos;
  gl_Position.y += sin(op.x + op.y) * sin(u_time) * 0.4;
  gl_Position.x += cos(op.x) * sin(u_time * 2) * 0.3;
  // gl_Position.z += sin(op.x) * sin(u_time);

#ifdef USE_CUBE_MAP
  gs_out.worldPos = gs_in[i].worldPos;
#endif
  gs_out.objectPos = gs_in[i].objectPos;
  gs_out.viewPos = gs_in[i].viewPos;
  gs_out.normal = gs_in[i].normal;
  gs_out.texCoord = gs_in[i].texCoord;

  gs_out.materialIndex = gs_in[i].materialIndex;
  gs_out.flags = gs_in[i].flags;

#ifdef USE_TBN
  gs_out.tbn = gs_in[i].tbn;
#endif
#ifdef USE_PARALLAX
  gs_out.viewTangentPos = gs_in[i].viewTangentPos;
  gs_out.tangentPos = gs_in[i].tangentPos;
#endif

#ifdef USE_BONES
#ifdef USE_DEBUG
  gs_out.boneBaseIndex = gs_in[i].boneBaseIndex;
  gs_out.boneIndex = gs_in[i].boneIndex;
  gs_out.boneWeight = gs_in[i].boneWeight;
  gs_out.boneColor = gs_in[i].boneColor;
#endif
#endif
#ifdef USE_DEBUG
  gs_out.socketIndex = gs_in[i].socketIndex;
#endif

  for (int ci = 0; ci < u_clipCount; ci++) {
    gl_ClipDistance[ci] = gl_in[ci].gl_ClipDistance[ci];
  }

  EmitVertex();
}

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  for (int i = 0; i < gs_in.length(); i++) {
    sendVertex(i, gl_in[i].gl_Position);
  }
  EndPrimitive();
}
