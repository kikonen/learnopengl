#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

#include struct_clip_plane.glsl

#include uniform_data.glsl
#include uniform_clip_planes.glsl

in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[CLIP_COUNT];
} gl_in[3];

in VS_OUT {
  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

#ifdef USE_TBN
  vec3 tangent;
#endif
} vs_in[];

out VS_OUT {
  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

#ifdef USE_TBN
  vec3 tangent;
#endif
} gs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;


vec3 getNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

vec4 explode(in vec4 pos, in vec3 normal)
{
  float magnitude = 2.0;
  float t = u_time;
  vec3 direction = normal * ((sin(t) + 1.0) / 2.0) * magnitude;
  return pos + vec4(direction, 0.0);
}

void sendVertex(in int i, in vec4 pos) {
  gl_Position = pos;

  gs_out.worldPos = vs_in[i].worldPos;
  gs_out.normal = vs_in[i].normal;
  gs_out.texCoord = vs_in[i].texCoord;
  gs_out.vertexPos = vs_in[i].vertexPos;

  gs_out.materialIndex = vs_in[i].materialIndex;

  gs_out.tangent = vs_in[i].tangent;

  for (int ci = 0; ci < u_clipCount; ci++) {
    gl_ClipDistance[ci] = gl_in[i].gl_ClipDistance[ci];
  }

  EmitVertex();
}

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  vec3 normal = getNormal();
  for (int i = 0; i < vs_in.length(); i++) {
    sendVertex(i, explode(gl_in[i].gl_Position, normal));
  }
  EndPrimitive();
}
