#version 460 core

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

#include uniform_data.glsl
#include uniform_matrices.glsl

in VS_OUT {
  flat vec4 objectID;

  vec3 scale;
  flat uint materialIndex;
} gs_in[];

out GS_OUT {
  flat vec4 objectID;

  vec2 texCoord;
  flat uint materialIndex;
} gs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void fillVertex(const int i)
{
  gs_out.materialIndex = gs_in[i].materialIndex;
  gs_out.objectID = gs_in[i].objectID;
}


void generateQuad(const int index)
{
  // https://ogldev.org/www/tutorial27/tutorial27.html
  vec3 pos = gl_in[0].gl_Position.xyz;
  vec3 scale = gs_in[index].scale * 2.0;
  vec3 viewDir = normalize(u_cameraPos.xyz - pos);
  vec3 up = vec3(0, 1, 0);
  vec3 right = cross(viewDir, up);

  vec3 scaledRight = right * scale;
  float scaledY = 1.0 * scale.y;

  vec4 worldPos;

  // bottom-left
  fillVertex(index);
  pos -= (scaledRight * 0.5);
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * worldPos;
  gs_out.texCoord = vec2(0.0, 0.0);

  EmitVertex();

  // top-left
  fillVertex(index);
  pos.y += scaledY;
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * worldPos;
  gs_out.texCoord = vec2(0.0, 1.0);

  EmitVertex();

  // bottom-right
  fillVertex(index);
  pos.y -= scaledY;
  pos += scaledRight;
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * worldPos;
  gs_out.texCoord = vec2(1.0, 0.0);

  EmitVertex();

  // top-right
  fillVertex(index);
  pos.y += scaledY;
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * worldPos;
  gs_out.texCoord = vec2(1.0, 1.0);

  EmitVertex();

  EndPrimitive();
}

void main() {
  for (int i = 0; i < gs_in.length(); i++) {
    generateQuad(i);
  }
}
