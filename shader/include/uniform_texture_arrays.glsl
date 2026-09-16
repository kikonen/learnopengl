// Global Samplers bound strictly to the shared configuration macro constants
layout(binding = UNIT_TEXTURE_ARRAY_SRGB)   uniform sampler2DArray u_texturesSRGB;
layout(binding = UNIT_TEXTURE_ARRAY_LINEAR) uniform sampler2DArray u_texturesLinear;
layout(binding = UNIT_TEXTURE_ARRAY_NORMAL) uniform sampler2DArray u_texturesNormal;
layout(binding = UNIT_TEXTURE_ARRAY_DUDV) uniform sampler2DArray u_texturesDudv;
layout(binding = UNIT_TEXTURE_ARRAY_NOISE)  uniform sampler2DArray u_texturesNoise;
layout(binding = UNIT_TEXTURE_ARRAY_DISPLACEMENT)  uniform sampler2DArray u_texturesDisplacement;
layout(binding = UNIT_TEXTURE_ARRAY_HEIGHT)  uniform sampler2DArray u_texturesHeight;
layout(binding = UNIT_TEXTURE_ARRAY_FONT_ATLAS)  uniform sampler2DArray u_texturesFontAtlas;
layout(binding = UNIT_TEXTURE_ARRAY_DYNAMIC)   uniform sampler2DArray u_texturesDynamic;
