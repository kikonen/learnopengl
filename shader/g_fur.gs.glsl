#version 460 core

layout (triangles) in;
// NOTE KI 73 == max what Nvidia GTX 1070 allowed
layout (triangle_strip, max_vertices = 64) out;

#include struct_clip_plane.glsl

#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_data.glsl
#include uniform_clip_planes.glsl

in VS_OUT {
  flat mat4 modelMatrix;

  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

  flat int layers;
  flat float layersDepth;
} vs_in[];

out VS_OUT {
  vec3 worldPos;
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;
  flat uint materialIndex;
  flat float furStrength;
} gs_out;


out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_calculate_clipping.glsl

void main() {
  const mat4 modelMatrix = vs_in[0].modelMatrix;
  const mat4 projectedModel = u_projectedMatrix * modelMatrix;

  const int furLayers = vs_in[0].layers;
  const float furDepth = vs_in[0].layersDepth;

  const float delta = 1.0 / float(furLayers);

  vec4 force = vec4(0, -0.05, 0, 0);
  force.x += sin(u_time / 2.0) / 16.0;
  force.y -= (1.0 + cos(u_time / 3.0)) / 32.0;
  force.z += cos(u_time / 2.0) / 32.0;

  for (int layer = 0; layer < furLayers; layer++) {
    const float d = delta * layer;

    for (int i = 0; i < gl_in.length(); i++) {
      vec3 normal = normalize(vs_in[i].normal);

      vec4 pos = gl_in[i].gl_Position + vec4(normal * d * furDepth, 0.0);
      // float scale = 1.0 + d * furDepth;
      // const mat4 scaleMat = mat4(scale, 0, 0, 0,
      //                            0, scale, 0, 0,
      //                            0, 0, scale, 0,
      //                            0, 0,     0, 1);
      // vec4 pos = scaleMat * gl_in[i].gl_Position;

      {
        // Couple of lines to give a swaying effect!
        // Additional Gravit/Force Code
        vec4 modelForce = modelMatrix * force;

        // We use the pow function, so that only the tips of the hairs bend
        // As layer goes from 0 to 1, so by using pow(..) function is still
        // goes form 0 to 1, but it increases faster! exponentially
        float k =  pow(d, 3);

        pos = pos + modelForce * k;
        // End Gravity Force Addit Code
      }

      vec4 worldPos = modelMatrix * pos;

      gs_out.furStrength = 1.0 - d;
      gs_out.worldPos = worldPos.xyx;
      gs_out.viewPos = (u_viewMatrix * worldPos).xyz;
      gs_out.normal = normal;
      gs_out.texCoord = vs_in[i].texCoord;
      gs_out.materialIndex = vs_in[i].materialIndex;
      gl_Position = projectedModel * pos;

      calculateClipping(worldPos);
      EmitVertex();
    }

    EndPrimitive();
  }
}
