#version 460 core

layout (triangles) in;
layout (line_strip, max_vertices = 120) out;

#include uniform_matrices.glsl

in VS_OUT {
  vec3 normal;
} vs_in[];

const float MAGNITUDE = 0.4;

void generateLine(int index)
{
  gl_Position = u_projectionMatrix * gl_in[index].gl_Position;
  EmitVertex();

  gl_Position = u_projectionMatrix * (gl_in[index].gl_Position +
                                    vec4(vs_in[index].normal, 0.0) * MAGNITUDE);
  EmitVertex();
  EndPrimitive();
}

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  for (int i = 0; i < vs_in.length(); i++) {
    generateLine(i);
  }
}