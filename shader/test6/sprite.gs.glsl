#version 450 core

#include constants.glsl

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

#include uniform_matrices.glsl

in VS_OUT {
  vec3 fragPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewVertexPos;

  flat uint materialIndex;

  vec4 fragPosLightSpace;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} vs_in[];

in VS_OUT {
  vec3 fragPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewVertexPos;

  flat uint materialIndex;

  vec4 fragPosLightSpace;
#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} gs_out;


void fillVertex(const int i)
{
  //gs_out.fragPos = vs_in[i].fragPos;
  gs_out.normal = vs_in[i].normal;
  //gs_out.texCoord = vs_in[i].texCoord;
  gs_out.vertexPos = vs_in[i].vertexPos;
  //gs_out.viewVertexPos = vs_in[i].viewVertexPos;
  gs_out.materialIndex = vs_in[i].materialIndex;
  //gs_out.fragPosLightSpace = vs_in[i].fragPosLightSpace;
#ifdef USE_NORMAL_TEX
  gs_out.TBN = vs_in[i].TBN;
#endif
}

void generateSprite(const int index)
{
  // https://ogldev.org/www/tutorial27/tutorial27.html
  vec3 toView = normalize(u_viewPos - gs_in[index].fragPos);
  vec3 up = vec3(0, 1, 0);
  vec3 right = cross(toView, up);

  vec3 pos;
  vec4 worldPos;

  fillVertex(index);
  pos -= (right * 0.5);
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * vec4(pos, 1.0);
  gs_out.fragPos = pos;
  gs_out.viewVertexPos = (u_viewMatrix * vec4(pos, 1.0)).xyz;
  gs_out.texCoord = vec2(0.0, 0.0);
  vs_out.fragPosLightSpace = u_shadowMatrix * worldPos;
  EmitVertex();

  fillVertex(index);
  pos.y += 1.0;
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * vec4(pos, 1.0);
  gs_out.fragPos = pos;
  gs_out.viewVertexPos = (u_viewMatrix * vec4(pos, 1.0)).xyz;
  gs_out.texCoord = vec2(0.0, 1.0);
  vs_out.fragPosLightSpace = u_shadowMatrix * worldPos;
  EmitVertex();

  fillVertex(index);
  pos.y -= 1.0;
  pos += right;
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * vec4(pos, 1.0);
  gs_out.fragPos = pos;
  gs_out.viewVertexPos = (u_viewMatrix * vec4(pos, 1.0)).xyz;
  gs_out.texCoord = vec2(1.0, 0.0);
  vs_out.fragPosLightSpace = u_shadowMatrix * worldPos;
  EmitVertex();

  fillVertex(index);
  pos.y += 1.0;
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * vec4(pos, 1.0);
  gs_out.fragPos = pos;
  gs_out.viewVertexPos = (u_viewMatrix * vec4(pos, 1.0)).xyz;
  gs_out.texCoord = vec2(1.0, 1.0);
  vs_out.fragPosLightSpace = u_shadowMatrix * worldPos;
  EmitVertex();

  EndPrimitive();
}

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  for (int i = 0; i < vs_in.length(); i++) {
    generateSprite(i);
  }
}
