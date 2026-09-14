#version 460 core

// NOTE KI screen space selection outline
// - reads silhouette mask rendered by LayerRenderer::fillHighlightMask
// - mask alpha != 0 == "inside silhouette", mask rgb == selection material color
// - replaces the old "4x shifted geometry redraw + stencil" approach, which
//   depended on stencil surviving the depth copy from gbuffer

// NOTE KI *NO* depth/stencil in this pass; outline is pure screen space
layout(early_fragment_tests) in;

layout(binding = UNIT_SELECTION_MASK) uniform sampler2D u_selectionMaskTex;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

// NOTE KI outline thickness in pixels; old shift approach was ~2px at z=1
const int OUTLINE_RADIUS = 2;

void main()
{
  const ivec2 texSize = textureSize(u_selectionMaskTex, 0);
  const ivec2 pixCoord = ivec2(gl_FragCoord.xy);

  // NOTE KI inside silhouette => object itself is drawn there, keep it visible
  if (texelFetch(u_selectionMaskTex, pixCoord, 0).a > 0.0)
    discard;

  vec4 outline = vec4(0.0);

  for (int y = -OUTLINE_RADIUS; y <= OUTLINE_RADIUS; y++) {
    for (int x = -OUTLINE_RADIUS; x <= OUTLINE_RADIUS; x++) {
      const ivec2 coord = clamp(pixCoord + ivec2(x, y), ivec2(0), texSize - 1);
      const vec4 sampled = texelFetch(u_selectionMaskTex, coord, 0);

      // NOTE KI pick strongest hit; all selected objects share selection material
      if (sampled.a > outline.a) {
        outline = sampled;
      }
    }
  }

  // NOTE KI no neighbour inside silhouette => not an outline pixel
  if (outline.a <= 0.0)
    discard;

  o_fragColor = vec4(outline.rgb, 1.0);
}
