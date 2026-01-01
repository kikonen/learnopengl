const ivec2 OUTLINE_OFFSETS[5] = ivec2[](
  ivec2(0, 0),
  ivec2(-1, -1),
  ivec2(-1, 1),
  ivec2(1, 1),
  ivec2(1, -1)
);

void renderOutline(const in int stencilMode)
{
  float w = 0.5;
  float pixelWidth = 2.0 / (u_bufferResolution.x * w);
  float pixelHeight = 2.0 / (u_bufferResolution.y * w);
  int offsetX = OUTLINE_OFFSETS[stencilMode].x;
  int offsetY = OUTLINE_OFFSETS[stencilMode].y;
  gl_Position.x += pixelWidth * gl_Position.z * offsetX;
  gl_Position.y += pixelHeight * gl_Position.z * offsetY;
}
