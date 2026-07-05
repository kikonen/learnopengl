// NOTE KI using vec4 split due to "alignment is biggest element size"
// => mat4 wastes lot of space (2 * vec4)
// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct Particle {
  float u_x;
  float u_y;
  float u_z;

  // uint u_materialIndex;
  // float u_scale;
  // uint u_spriteIndex;

  // material = 16bit
  // scale = 8 bit
  // index = 8 bit
  uint u_msp;
};

uint unpackParticleIndex(uint msp) {
  return (msp >> 16);
}

float unpackParticleScale(uint msp) {
  return (float((msp >> 8) & 255) / 255.0) * PARTICLE_MAX_SCALE;
}

uint unpackParticleSpriteIndex(uint msp) {
  return msp & 255;;
}
