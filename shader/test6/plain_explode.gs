#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

#include uniform_data.glsl

in VS_OUT {
  flat float materialIndex;
  vec3 fragPos;
  vec3 normal;
} vs_in[];

out VS_OUT {
  vec3 fragPos;

  flat float materialIndex;
  vec3 normal;
} gs_out;

vec3 getNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

vec4 explode(vec4 pos, vec3 normal)
{
  float magnitude = 2.0;
  float t = time;
  vec3 direction = normal * ((sin(t) + 1.0) / 2.0) * magnitude;
  return pos + vec4(direction, 0.0);
}

void sendVertex(int i, vec4 pos) {
  gl_Position = pos;
  gs_out.materialIndex = vs_in[i].materialIndex;
  gs_out.fragPos = vs_in[i].fragPos;
  gs_out.normal = vs_in[i].normal;
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
