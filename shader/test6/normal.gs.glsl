#version 450 core

#include constants.glsl

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

#include uniform_matrices.glsl

in VS_OUT {
  vec3 normal;
} vs_in[];

const float MAGNITUDE = 0.2;

void generateLine(const int index)
{
  gl_Position = projectionMatrix * gl_in[index].gl_Position;
  EmitVertex();

  gl_Position = projectionMatrix * (gl_in[index].gl_Position +
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
