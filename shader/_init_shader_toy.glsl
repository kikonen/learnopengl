iTime = u_time;
iFrame = u_frame;
iResolution = u_bufferResolution;
iMaterial = material;

iChannel0 = sampler2D(u_materials[materialIndex].noiseMapTex);
iChannel2 = sampler2D(u_materials[materialIndex].noise2MapTex);
