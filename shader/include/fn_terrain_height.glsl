// Jolt-compatible height sampling using triangulated interpolation
// Matches Jolt HeightFieldShape's triangle-based height calculation
float fetchHeight(
  in sampler2D heightMap,
  in vec2 texCoord)
{
  // Get texture dimensions
  ivec2 texSize = textureSize(heightMap, 0);

  // Convert normalized coords to texel space
  vec2 texelCoord = texCoord * vec2(texSize) - 0.5;

  // Get integer cell coordinates
  ivec2 cell = ivec2(floor(texelCoord));

  // Fractional position within cell [0,1]
  vec2 f = texelCoord - vec2(cell);

  // Clamp cell coordinates to valid range
  cell = clamp(cell, ivec2(0), texSize - 2);

  // Fetch 4 corner heights (no filtering)
  float h00 = texelFetch(heightMap, cell + ivec2(0, 0), 0).r;
  float h10 = texelFetch(heightMap, cell + ivec2(1, 0), 0).r;
  float h01 = texelFetch(heightMap, cell + ivec2(0, 1), 0).r;
  float h11 = texelFetch(heightMap, cell + ivec2(1, 1), 0).r;

  // Jolt uses checkered diagonal pattern: (x^z)&1 determines diagonal
  // When pattern bit is 0: diagonal from (0,0) to (1,1)
  // When pattern bit is 1: diagonal from (1,0) to (0,1)
  bool diagPattern = ((cell.x ^ cell.y) & 1) != 0;

  float height;
  if (diagPattern) {
    // Diagonal from (1,0) to (0,1)
    if (f.x + f.y < 1.0) {
      // Lower-left triangle: vertices (0,0), (1,0), (0,1)
      height = h00 + f.x * (h10 - h00) + f.y * (h01 - h00);
    } else {
      // Upper-right triangle: vertices (1,1), (0,1), (1,0)
      height = h11 + (1.0 - f.x) * (h01 - h11) + (1.0 - f.y) * (h10 - h11);
    }
  } else {
    // Diagonal from (0,0) to (1,1)
    if (f.x > f.y) {
      // Lower-right triangle: vertices (0,0), (1,0), (1,1)
      height = h00 + f.x * (h10 - h00) + f.y * (h11 - h10);
    } else {
      // Upper-left triangle: vertices (0,0), (0,1), (1,1)
      height = h00 + f.x * (h11 - h01) + f.y * (h01 - h00);
    }
  }

  return height;
}
