// NOTE KI GL_RGB16F
// => *MUST* match FrameBufferAttachment::getGBufferNormal
// NOTE KI "var" approach since fn approach produced garbage in intel GPU
//const vec3 normal = texture(g_normal, texCoord).xyz * 2.0 - 1.0;
//const vec3 normal = texture(g_normal, texCoord).xyz;
const vec3 normal = decodeGNormal(texCoord, viewPos);
