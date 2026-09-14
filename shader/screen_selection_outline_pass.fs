#version 460 core

// NOTE KI screen space selection outline
// - reads silhouette mask rendered by LayerRenderer::fillHighlightMask
// - mask alpha != 0 == "inside silhouette", mask rgb == selection material color
// - replaces the old "4x shifted geometry redraw + stencil" approach, which
//   depended on stencil surviving the depth copy from gbuffer
// - mask is rendered at lower res than this pass; LINEAR sampling interpolates
//   the silhouette edge, which is what makes the outline anti-aliased

// NOTE KI *NO* depth/stencil in this pass; outline is pure screen space
layout(early_fragment_tests) in;

#include "include/screen_tri_vertex_out.glsl"

layout(binding = UNIT_SELECTION_MASK) uniform sampler2D u_selectionMaskTex;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

// NOTE KI thickness in *mask* texels; everything is derived from the mask
// itself, thus no dependency on u_bufferResolution (which is not even declared
// here, since globals.glsl defines SCREEN_TRI_VERTEX_OUT for all shaders).
// => effective thickness scales with SELECTION_MASK_SCALE in LayerRenderer
const float OUTLINE_WIDTH = 1.25;

const vec2 OUTLINE_DIRS[8] = vec2[](
  vec2( 1.0,  0.0),
  vec2(-1.0,  0.0),
  vec2( 0.0,  1.0),
  vec2( 0.0, -1.0),
  vec2( 0.7071,  0.7071),
  vec2(-0.7071,  0.7071),
  vec2( 0.7071, -0.7071),
  vec2(-0.7071, -0.7071)
);

void main()
{
  #include "include/screen_tri_tex_coord.glsl"

  const vec2 stepUv = OUTLINE_WIDTH / vec2(textureSize(u_selectionMaskTex, 0));

  // NOTE KI fractional at silhouette edge thanks to LINEAR
  const float inside = texture(u_selectionMaskTex, texCoord).a;

  vec4 nearest = vec4(0.0);

  // NOTE KI two radii per direction; single ring leaves gaps on thin geometry
  for (int i = 0; i < 8; i++) {
    for (int r = 1; r <= 2; r++) {
      const vec2 offset = OUTLINE_DIRS[i] * stepUv * (float(r) * 0.5);
      const vec4 sampled = texture(u_selectionMaskTex, texCoord + offset);

      if (sampled.a > nearest.a) {
        nearest = sampled;
      }
    }
  }

  // NOTE KI ring == covered by neighbourhood, but not by silhouette itself.
  // Both terms are fractional at the edges => anti-aliased inner *and* outer
  // border; composited with regular src-alpha blend by the caller.
  const float coverage = nearest.a * (1.0 - inside);

  if (coverage <= 0.0)
    discard;

  // NOTE KI mask is *not* premultiplied; LINEAR interpolates rgb towards the
  // cleared (0,0,0,0) at the silhouette edge, which would darken the outline.
  // Undo that by normalizing with the sampled alpha.
  const vec3 color = nearest.rgb / nearest.a;

  o_fragColor = vec4(color, coverage);
}
