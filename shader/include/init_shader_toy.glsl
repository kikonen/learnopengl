iTime = u_time;
iFrame = u_frame;
iResolution = u_bufferResolution;
iMaterial = material;

iChannel0 = sampler2D(readMaterialNoiseTexTBO(materialIndex));
iChannel2 = sampler2D(readMaterialNoise2TexTBO(materialIndex));
