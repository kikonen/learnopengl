#version 460 core

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

#include struct_clip_plane.glsl

#include uniform_data.glsl
#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

in VS_OUT {
  flat uint entityIndex;

  vec3 normal;
  vec3 vertexPos;

  flat uint materialIndex;

  vec3 scale;

#ifdef USE_TBN
  vec3 tangent;
#endif
} gs_in[];

out GS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  flat uint shadowIndex;
  vec4 shadowPos;

#ifdef USE_TBN
  vec3 tangent;
#endif
} gs_out;


out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_calculate_clipping.glsl
#include fn_calculate_shadow_index.glsl


void fillVertex(const int i)
{
  gs_out.entityIndex = gs_in[i].entityIndex;

  gs_out.normal = gs_in[i].normal;
  gs_out.vertexPos = gs_in[i].vertexPos;
  gs_out.materialIndex = gs_in[i].materialIndex;
#ifdef USE_TBN
  gs_out.tangent = gs_in[i].tangent;
#endif
}

void generateQuad(const int index)
{
  // https://ogldev.org/www/tutorial27/tutorial27.html
  vec3 pos = gl_in[0].gl_Position.xyz;
  vec3 scale = gs_in[index].scale * 2.0;
  vec3 viewDir = normalize(u_cameraPos - pos);
  vec3 up = vec3(0, 1, 0);
  vec3 right = cross(viewDir, up);

  vec3 scaledRight = right * scale;
  float scaledY = 1.0 * scale.y;

  vec4 worldPos;
  vec3 viewPos;
  uint shadowIndex;

  // bottom-left
  fillVertex(index);
  pos -= (scaledRight * 0.5);

  worldPos = vec4(pos, 1.0);
  viewPos = (u_viewMatrix * worldPos).xyz;
  shadowIndex = calculateShadowIndex(viewPos);

  gl_Position = u_projectedMatrix * worldPos;
  gs_out.worldPos = worldPos;
  gs_out.viewPos = (u_viewMatrix * worldPos).xyz;
  gs_out.texCoord = vec2(0.0, 0.0);

  gs_out.shadowIndex = shadowIndex;
  gs_out.shadowPos = u_shadowMatrix[shadowIndex] * worldPos;

  calculateClipping(worldPos);
  EmitVertex();

  // top-left
  fillVertex(index);
  pos.y += scaledY;

  worldPos = vec4(pos, 1.0);
  viewPos = (u_viewMatrix * worldPos).xyz;
  shadowIndex = calculateShadowIndex(viewPos);

  gl_Position = u_projectedMatrix * worldPos;
  gs_out.worldPos = worldPos;
  gs_out.viewPos = (u_viewMatrix * worldPos).xyz;
  gs_out.texCoord = vec2(0.0, 1.0);

  gs_out.shadowIndex = shadowIndex;
  gs_out.shadowPos = u_shadowMatrix[shadowIndex] * worldPos;

  calculateClipping(worldPos);
  EmitVertex();

  // bottom-right
  fillVertex(index);
  pos.y -= scaledY;
  pos += scaledRight;

  worldPos = vec4(pos, 1.0);
  viewPos = (u_viewMatrix * worldPos).xyz;
  shadowIndex = calculateShadowIndex(viewPos);

  gl_Position = u_projectedMatrix * worldPos;
  gs_out.worldPos = worldPos;
  gs_out.viewPos = (u_viewMatrix * worldPos).xyz;
  gs_out.texCoord = vec2(1.0, 0.0);

  gs_out.shadowIndex = shadowIndex;
  gs_out.shadowPos = u_shadowMatrix[shadowIndex] * worldPos;

  calculateClipping(worldPos);
  EmitVertex();

  // top-right
  fillVertex(index);
  pos.y += scaledY;

  worldPos = vec4(pos, 1.0);
  viewPos = (u_viewMatrix * worldPos).xyz;
  shadowIndex = calculateShadowIndex(viewPos);

  gl_Position = u_projectedMatrix * worldPos;
  gs_out.worldPos = worldPos;
  gs_out.viewPos = (u_viewMatrix * worldPos).xyz;
  gs_out.texCoord = vec2(1.0, 1.0);

  gs_out.shadowIndex = shadowIndex;
  gs_out.shadowPos = u_shadowMatrix[shadowIndex] * worldPos;

  calculateClipping(worldPos);
  EmitVertex();

  EndPrimitive();
}

void main() {
  for (int i = 0; i < gs_in.length(); i++) {
    generateQuad(i);
  }
}
