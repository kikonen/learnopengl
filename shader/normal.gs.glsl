#version 460 core

layout (triangles) in;
layout (line_strip, max_vertices = 12) out;

#include uniform_matrices.glsl

in VS_OUT {
  vec3 normal;
  flat mat3 TBN;
} vs_in[];

out GS_OUT {
  vec4 fragColor;
} gs_out;

const float MAGNITUDE = 0.2;

void generateNormal(const int index)
{
  vec3 normal = normalize(vs_in[index].normal);

  gs_out.fragColor = vec4(1.0, 1.0, 0.0, 1.0);

  gl_Position = u_projectionMatrix * gl_in[index].gl_Position;
  EmitVertex();

  gs_out.fragColor = vec4(1.0, 1.0, 0.0, 1.0);

  gl_Position = u_projectionMatrix * (gl_in[index].gl_Position +
                                      vec4(normal, 0.0) * MAGNITUDE);
  EmitVertex();
  EndPrimitive();
}

void generateTangent(const int index)
{
  vec3 tangent = vs_in[index].TBN[0];

  gs_out.fragColor = vec4(0.0, 0.0, 1.0, 1.0);

  gl_Position = u_projectionMatrix * gl_in[index].gl_Position;
  EmitVertex();

  gs_out.fragColor = vec4(0.0, 0.0, 1.0, 1.0);

  gl_Position = u_projectionMatrix * (gl_in[index].gl_Position +
                                      vec4(tangent, 0.0) * MAGNITUDE);
  EmitVertex();
  EndPrimitive();
}

void generateBitangent(const int index)
{
  vec3 bitangent = vs_in[index].TBN[1];

  gs_out.fragColor = vec4(0.0, 1.0, 0.0, 1.0);

  gl_Position = u_projectionMatrix * gl_in[index].gl_Position;
  EmitVertex();

  gs_out.fragColor = vec4(0.0, 1.0, 0.0, 1.0);

  gl_Position = u_projectionMatrix * (gl_in[index].gl_Position +
                                      vec4(bitangent, 0.0) * MAGNITUDE);
  EmitVertex();
  EndPrimitive();
}

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  for (int i = 0; i < vs_in.length(); i++) {
    generateNormal(i);
    generateTangent(i);
    generateBitangent(i);
  }
}
