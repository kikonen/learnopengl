#ifdef USE_TEXTURE_ARRAY
// Global Samplers bound strictly to the shared configuration macro constants
layout(binding = UNIFORM_TEXTURE_ARRAY_SRGB)   uniform sampler2DArray u_TexturesSRGB;
layout(binding = UNIFORM_TEXTURE_ARRAY_LINEAR) uniform sampler2DArray u_TexturesLinear;
layout(binding = UNIFORM_TEXTURE_ARRAY_NORMAL) uniform sampler2DArray u_TexturesNormal;
layout(binding = UNIFORM_TEXTURE_ARRAY_NOISE)  uniform sampler2DArray u_TexturesNoise;
#endif
