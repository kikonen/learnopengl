#version 450 core

#include constants.glsl

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

#include struct_clip_plane.glsl

#include uniform_data.glsl
#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

in VS_OUT {
  vec3 fragPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewVertexPos;

  flat uint materialIndex;

  vec4 fragPosLightSpace;

  vec3 scale;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} gs_in[];

out GS_OUT {
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


out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_clipping.glsl

void fillVertex(const int i)
{
  gs_out.normal = gs_in[i].normal;
  gs_out.vertexPos = gs_in[i].vertexPos;
  gs_out.materialIndex = gs_in[i].materialIndex;
#ifdef USE_NORMAL_TEX
  gs_out.TBN = gs_in[i].TBN;
#endif
}

void generateSprite(const int index)
{
  // https://ogldev.org/www/tutorial27/tutorial27.html
  vec3 pos = gl_in[0].gl_Position.xyz;
  vec3 scale = gs_in[index].scale;
  vec3 toView = normalize(u_viewPos - pos);
  vec3 up = vec3(0, 1, 0);
  vec3 right = cross(toView, up);

  vec4 worldPos;

  fillVertex(index);
  pos -= (right * 0.5) * scale;
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * worldPos;
  gs_out.fragPos = pos;
  gs_out.viewVertexPos = (u_viewMatrix * worldPos).xyz;
  gs_out.texCoord = vec2(0.0, 0.0);
  gs_out.fragPosLightSpace = u_shadowMatrix * worldPos;

  calculateClipping(worldPos);
  EmitVertex();

  fillVertex(index);
  pos.y += 1.0 * scale.y;
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * worldPos;
  gs_out.fragPos = pos;
  gs_out.viewVertexPos = (u_viewMatrix * worldPos).xyz;
  gs_out.texCoord = vec2(0.0, 1.0);
  gs_out.fragPosLightSpace = u_shadowMatrix * worldPos;

  calculateClipping(worldPos);
  EmitVertex();

  fillVertex(index);
  pos.y -= 1.0 * scale.y;
  pos += right * scale;
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * worldPos;
  gs_out.fragPos = pos;
  gs_out.viewVertexPos = (u_viewMatrix * worldPos).xyz;
  gs_out.texCoord = vec2(1.0, 0.0);
  gs_out.fragPosLightSpace = u_shadowMatrix * worldPos;

  calculateClipping(worldPos);
  EmitVertex();

  fillVertex(index);
  pos.y += 1.0 * scale.y;
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * worldPos;
  gs_out.fragPos = pos;
  gs_out.viewVertexPos = (u_viewMatrix * worldPos).xyz;
  gs_out.texCoord = vec2(1.0, 1.0);
  gs_out.fragPosLightSpace = u_shadowMatrix * worldPos;

  calculateClipping(worldPos);
  EmitVertex();

  EndPrimitive();
}

void main() {
  for (int i = 0; i < gs_in.length(); i++) {
    generateSprite(i);
  }
}
