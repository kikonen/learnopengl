#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_data.glsl

in gl_PerVertex
{
  vec4 gl_Position;
#ifdef USE_GL_POINTS
  float gl_PointSize;
#endif
  float gl_ClipDistance[CLIP_COUNT];
} gl_in[3];

in VS_OUT {
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 objectPos;
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
  flat uint flags;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif

#ifdef USE_BONES
#ifdef USE_DEBUG
  flat uint boneBaseIndex;
  flat uvec4 boneIndex;
  flat vec4 boneWeight;
  vec3 boneColor;
#endif
#endif
#ifdef USE_DEBUG
  flat uint socketIndex;
#endif
} gs_in[];

out VS_OUT {
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 objectPos;
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
  flat uint flags;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif

#ifdef USE_BONES
#ifdef USE_DEBUG
  flat uint boneBaseIndex;
  flat uvec4 boneIndex;
  flat vec4 boneWeight;
  vec3 boneColor;
#endif
#endif
#ifdef USE_DEBUG
  flat uint socketIndex;
#endif

  noperspective vec3 edgeDistance;
} gs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void emitVertex(const uint i)
{
#ifdef USE_CUBE_MAP
  gs_out.worldPos = gs_in[i].worldPos;
#endif
  gs_out.objectPos = gs_in[i].objectPos;
  gs_out.viewPos = gs_in[i].viewPos;
  gs_out.normal = gs_in[i].normal;
  gs_out.texCoord = gs_in[i].texCoord;

  gs_out.materialIndex = gs_in[i].materialIndex;
  gs_out.flags = gs_in[i].flags;

#ifdef USE_TBN
  gs_out.tbn = gs_in[i].tbn;
#endif
#ifdef USE_PARALLAX
  gs_out.viewTangentPos = gs_in[i].viewTangentPos;
  gs_out.tangentPos = gs_in[i].tangentPos;
#endif

#ifdef USE_BONES
#ifdef USE_DEBUG
  gs_out.boneBaseIndex = gs_in[i].boneBaseIndex;
  gs_out.boneIndex = gs_in[i].boneIndex;
  gs_out.boneWeight = gs_in[i].boneWeight;
  gs_out.boneColor = gs_in[i].boneColor;
#endif
#endif
#ifdef USE_DEBUG
  gs_out.socketIndex = gs_in[i].socketIndex;
#endif

  gl_Position = gl_in[i].gl_Position;

  for (int ci = 0; ci < CLIP_COUNT; ci++) {
    gl_ClipDistance[ci] = gl_in[i].gl_ClipDistance[ci];
  }

  EmitVertex();
}

void main() {
  // Transform each vertex into viewport space
  const vec3 p0 = vec3(u_viewportMatrix * (gl_in[0].gl_Position /
					   gl_in[0].gl_Position.w));
  const vec3 p1 = vec3(u_viewportMatrix * (gl_in[1].gl_Position /
					   gl_in[1].gl_Position.w));
  const vec3 p2 = vec3(u_viewportMatrix * (gl_in[2].gl_Position /
					   gl_in[2].gl_Position.w));

  // Find the altitudes (ha, hb and hc)
  const float a = length(p1 - p2);
  const float b = length(p2 - p0);
  const float c = length(p1 - p0);
  const float alpha = acos( (b * b + c * c - a * a) / (2.0 * b * c) );
  const float beta = acos( (a * a + c * c - b * b) / (2.0 * a * c) );
  const float ha = abs( c * sin( beta ) );
  const float hb = abs( c * sin( alpha ) );
  const float hc = abs( b * sin( alpha ) );

  // Send the triangle along with the edge distances
  gs_out.edgeDistance = vec3( ha, 0, 0 );
  emitVertex(0);

  gs_out.edgeDistance = vec3( 0, hb, 0 );
  emitVertex(1);

  gs_out.edgeDistance = vec3( 0, 0, hc );
  emitVertex(2);

  EndPrimitive();
}
