#ifdef SCREEN_TRI_VERTEX_OUT
const vec2 texCoord = fs_in.texCoord;
#else
// NOTE KI pixCoord == texCoord in fullscreen quad
const vec2 texCoord = gl_FragCoord.xy / u_bufferResolution;
#endif
