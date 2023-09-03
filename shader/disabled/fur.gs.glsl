#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 128) out;

#include uniform_matrices.glsl
#include uniform_data.glsl

in VS_OUT {
  flat mat4 modelMatrix;

  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

  flat int layers;
  flat float layersDepth;
} vs_in[];

out VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
  flat float furStrength;
} gs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

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
      vec3 n = normalize(vs_in[i].normal);

      vec4 pos = gl_in[i].gl_Position + vec4(n * d * furDepth, 0.0);
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

      gs_out.furStrength = 1.0 - d;
      gs_out.texCoord = vs_in[i].texCoord;
      gs_out.materialIndex = vs_in[i].materialIndex;
      gl_Position = projectedModel * pos;

      EmitVertex();
    }

    EndPrimitive();
  }
}
