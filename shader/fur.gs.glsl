#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 240) out;

#include uniform_matrices.glsl
#include uniform_data.glsl

in VS_OUT {
  mat4 modelMatrix;

  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

  flat int layers;
  flat float depth;
} vs_in[];

out VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
  flat float furStrength;
} gs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  const int furLayers = vs_in[0].layers;
  const float furDepth = vs_in[0].depth;

  float delta = 1.0 / float(furLayers);
  float d = 0.0;

  for (int layer = 0; layer < furLayers; layer++) {
    for (int i = 0; i < gl_in.length(); i++) {
      vec3 n = normalize(vs_in[i].normal);

     vec4 pos = gl_in[i].gl_Position + vec4(n * d * furDepth, 0.0);
      // float scale = 1.0 + d * furDepth;
      // const mat4 scaleMat = mat4(scale, 0, 0, 0,
      //                            0, scale, 0, 0,
      //                            0, 0, scale, 0,
      //                            0, 0,     0, 1);
      // vec4 pos = scaleMat * gl_in[i].gl_Position;

      //      pos.x += (sin(u_time + d) / 10.0);
//      pos.y += (sin(u_time * d * 1.2) / 12.0);

      gs_out.furStrength = 1.0 - d;
      gs_out.texCoord = vs_in[i].texCoord;
      gs_out.materialIndex = vs_in[i].materialIndex;
      gl_Position = u_projectedMatrix * vs_in[i].modelMatrix * pos;

      EmitVertex();
    }
    d += delta;

    EndPrimitive();
  }
}
