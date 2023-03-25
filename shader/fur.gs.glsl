#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 120) out;

#include uniform_matrices.glsl
#include uniform_data.glsl

in VS_OUT {
  mat4 modelMatrix;
  mat3 normalMatrix;

  vec3 normal;
#ifdef USE_NORMAL_TEX
  vec3 tangent;
#endif
  vec2 texCoord;

  flat uint materialIndex;
} vs_in[];

out VS_OUT {
  vec4 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;
  flat float furStrength;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} gs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

const uint LAYERS = 10;

vec3 getNormal(in int i)
{
  return normalize(vs_in[i].normalMatrix * vs_in[i].normal);
}

vec3 getTangent(in int i)
{
  return normalize((vs_in[i].modelMatrix * vec4(vs_in[i].tangent, 1.0)).xyz);
}

vec4 shell(in vec4 pos, in int layer, in vec3 normal, in vec3 tangent)
{
  const float magnitude = 0.02;
  const float scale = 1.0 + layer * magnitude;

  const mat4 scaleMat = mat4(scale, 0, 0, 0,
                             0, scale, 0, 0,
                             0, 0, scale, 0,
                             0, 0,     0, 1);
  vec4 furPos = scaleMat * pos;

  float gravity = 0.3;
  float wind = 0.01;
  float t = u_time;
  vec3 direction = tangent * sin(t) * wind * layer;
//  furPos += vec4(direction, 0.0);
//  furPos.y -= sin(float(layer) / float(LAYERS)) * gravity;
  return furPos;
}

void sendVertex(in int i, in uint layer, in vec4 pos) {
  gs_out.furStrength = 1.0 - float(layer) / float(LAYERS);
  gs_out.worldPos = vs_in[i].modelMatrix * pos;
  gs_out.normal = normalize(vs_in[i].normalMatrix * vs_in[i].normal);
  gs_out.texCoord = vs_in[i].texCoord;
  gs_out.vertexPos = gl_in[i].gl_Position.xyz;

  gs_out.materialIndex = vs_in[i].materialIndex;

  gl_Position = u_projectedMatrix * gs_out.worldPos;

#ifdef USE_NORMAL_TEX
  if (false)
  {
    const vec3 N = normalize(gs_out.normal);
    vec3 T = normalize(vs_in[i].modelMatrix * vec4(vs_in[i].tangent, 1.0)).xyz;
    T = normalize(T - dot(T, N) * N);
    const vec3 B = cross(N, T);

    gs_out.TBN = mat3(T, B, N);
  }
#endif

  EmitVertex();
}

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  for (int layer = 0; layer < LAYERS; layer++) {
    for (int i = 0; i < vs_in.length(); i++) {
      sendVertex(i, layer, shell(gl_in[i].gl_Position, layer, getNormal(i), getTangent(i)));
    }
    EndPrimitive();
  }
}
