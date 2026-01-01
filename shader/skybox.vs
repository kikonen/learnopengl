#version 460 core

#include include/texture_quad.glsl

#include include/uniform_matrices.glsl
#include include/uniform_camera.glsl
#include include/uniform_data.glsl

out VS_OUT {
  vec3 texCoord;
} vs_out;

out float gl_ClipDistance[2];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  vec4 pos = vec4(VERTEX_POS[gl_VertexID], 1.0);
  pos.z = 1.0;

  // https://www.rioki.org/2013/03/07/glsl-skybox.html
  // mat4 m = u_viewMatrix;
  // m[3][0] = 0.0;
  // m[3][2] = 0.0;
  // m[3][2] = 0.0;
  // vec4 v = inverse(m) * inverse(u_projectionMatrix) * pos;

  vec4 v = u_viewMatrixSkybox * pos;
  // v.xyz = rotateEuler(v.xyz, vec3(0, u_time * 2, u_time * 10));

  gl_Position = pos.xyww;
  vs_out.texCoord = v.xyz;

  gl_ClipDistance[0] = 1;
  gl_ClipDistance[1] = 1;
}
