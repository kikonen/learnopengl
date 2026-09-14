iTime = u_time;
iFrame = u_frame;
iResolution = u_bufferResolution;
iMaterial = material;

#ifndef USE_TEXTURE_ARRAY
iChannel0 = sampler2D(readMaterial_noiseMapTex(materialIndex));
iChannel2 = sampler2D(readMaterial_noise2MapTex(materialIndex));
#endif

#ifdef USE_TEXTURE_ARRAY
// TODO KI these are not used; but need to us 2darray or uniforms somehow
// iChannel0 = sampler2D(readMaterial_noiseMapTex(materialIndex));
// iChannel2 = sampler2D(readMaterial_noise2MapTex(materialIndex));
#endif
