#version 460 core

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

#include struct_clip_plane.glsl

#include uniform_data.glsl
#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

in VS_OUT {
  vec3 normal;
  vec4 vertexPos;

  flat uint materialIndex;

  vec3 scale;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} gs_in[];

out GS_OUT {
  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec4 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  vec4 shadowPos;
#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} gs_out;


out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

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

void generateQuad(const int index)
{
  // https://ogldev.org/www/tutorial27/tutorial27.html
  vec3 pos = gl_in[0].gl_Position.xyz;
  vec3 scale = gs_in[index].scale * 2.0;
  vec3 toView = normalize(u_viewWorldPos - pos);
  vec3 up = vec3(0, 1, 0);
  vec3 right = cross(toView, up);

  vec3 scaledRight = right * scale;
  float scaledY = 1.0 * scale.y;

  vec4 worldPos;

  // bottom-left
  fillVertex(index);
  pos -= (scaledRight * 0.5);
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * worldPos;
  gs_out.worldPos = worldPos;
  gs_out.viewPos = (u_viewMatrix * worldPos).xyz;
  gs_out.texCoord = vec2(0.0, 0.0);
  gs_out.shadowPos = u_shadowMatrix * worldPos;

  calculateClipping(worldPos);
  EmitVertex();

  // top-left
  fillVertex(index);
  pos.y += scaledY;
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * worldPos;
  gs_out.worldPos = worldPos;
  gs_out.viewPos = (u_viewMatrix * worldPos).xyz;
  gs_out.texCoord = vec2(0.0, 1.0);
  gs_out.shadowPos = u_shadowMatrix * worldPos;

  calculateClipping(worldPos);
  EmitVertex();

  // bottom-right
  fillVertex(index);
  pos.y -= scaledY;
  pos += scaledRight;
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * worldPos;
  gs_out.worldPos = worldPos;
  gs_out.viewPos = (u_viewMatrix * worldPos).xyz;
  gs_out.texCoord = vec2(1.0, 0.0);
  gs_out.shadowPos = u_shadowMatrix * worldPos;

  calculateClipping(worldPos);
  EmitVertex();

  // top-right
  fillVertex(index);
  pos.y += scaledY;
  worldPos = vec4(pos, 1.0);
  gl_Position = u_projectedMatrix * worldPos;
  gs_out.worldPos = worldPos;
  gs_out.viewPos = (u_viewMatrix * worldPos).xyz;
  gs_out.texCoord = vec2(1.0, 1.0);
  gs_out.shadowPos = u_shadowMatrix * worldPos;

  calculateClipping(worldPos);
  EmitVertex();

  EndPrimitive();
}

void main() {
  for (int i = 0; i < gs_in.length(); i++) {
    generateQuad(i);
  }
}
