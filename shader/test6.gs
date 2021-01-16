#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT {
  flat float materialIndex;
  vec3 fragPos;
  vec3 normal;
} vs_in[];

flat out float materialIndex;
out vec3 fragPos;
out vec3 normal;

void sendVertex(int i) {
  gl_Position = gl_in[i].gl_Position;
  materialIndex = vs_in[i].materialIndex;
  fragPos = vs_in[i].fragPos;
  normal = vs_in[i].normal;
  EmitVertex();
}

void main() {
  for (int i = 0; i < vs_in.length(); i++) {
    sendVertex(i);
  }
  EndPrimitive();
}
