#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT {
  flat float materialIndex;
  vec3 fragPos;
  vec3 normal;

  vec4 fragPosLightSpace;
} vs_in[];

out VS_OUT {
  vec3 fragPos;

  flat float materialIndex;
  vec3 normal;

  vec4 fragPosLightSpace;
} gs_out;

void sendVertex(int i) {
  gl_Position = gl_in[i].gl_Position;
  gs_out.materialIndex = vs_in[i].materialIndex;
  gs_out.fragPos = vs_in[i].fragPos;
  gs_out.normal = vs_in[i].normal;
  gs_out.fragPosLightSpace = vs_in[i].fragPosLightSpace;
  EmitVertex();
}

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  for (int i = 0; i < vs_in.length(); i++) {
    sendVertex(i);
  }
  EndPrimitive();
}
