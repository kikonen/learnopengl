iTime = u_time;
iFrame = u_frame;
iResolution = u_bufferResolution;
iMaterial = material;

iChannel0 = sampler2D(readMaterial_noiseMapTex(materialIndex));
iChannel2 = sampler2D(readMaterial_noise2MapTex(materialIndex));
