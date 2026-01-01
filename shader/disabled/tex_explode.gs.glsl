#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

#include include/uniform_data.glsl

in VS_OUT {
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

  vec3 worldPos;
  vec3 normal;

  vec4 shadowPos;

#ifdef USE_TBN
  vec3 tangent;
#endif
} vs_in[];

out VS_OUT {
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

  vec3 worldPos;
  vec3 normal;

  vec4 shadowPos;

#ifdef USE_TBN
  vec3 tangent;
#endif
} gs_out;

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
  gs_out.texCoord = vs_in[i].texCoord;
  gs_out.vertexPos = vs_in[i].vertexPos;
  gs_out.materialIndex = vs_in[i].materialIndex;
  gs_out.worldPos = vs_in[i].worldPos;
  gs_out.normal = vs_in[i].normal;
  gs_out.shadowPos = vs_in[i].shadowPos;
  gs_out.tangent = vs_in[i].tangent;
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
