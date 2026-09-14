vec2 iResolution;
float iTime;
int iFrame;
float iTimeDelta = 0.0;
vec4 iMouse = vec4(0);

#ifndef USE_TEXTURE_ARRAY
sampler2D iChannel0;
sampler2D iChannel1;
sampler2D iChannel2;
sampler2D iChannel3;
#endif

#ifdef USE_TEXTURE_ARRAY
// TODO KI these are not used; but need to us 2darray or uniforms somehow
// sampler2D iChannel0;
// sampler2D iChannel1;
// sampler2D iChannel2;
// sampler2D iChannel3;
#endif

ResolvedMaterial iMaterial;
