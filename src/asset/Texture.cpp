#include "Texture.h"

#include <mutex>

#include "Program.h"

#include "TextureUBO.h"


namespace {
    //const int UNIT_IDS[] = {
    //    GL_TEXTURE0,
    //    GL_TEXTURE1,
    //    GL_TEXTURE2,
    //    GL_TEXTURE3,
    //    GL_TEXTURE4,
    //    GL_TEXTURE5,
    //    GL_TEXTURE6,
    //    GL_TEXTURE7,
    //    GL_TEXTURE8,
    //    GL_TEXTURE9,
    //    GL_TEXTURE10,
    //    GL_TEXTURE11,
    //    GL_TEXTURE12,
    //    GL_TEXTURE13,
    //    GL_TEXTURE14,
    //    GL_TEXTURE15,
    //    GL_TEXTURE16,
    //    GL_TEXTURE17,
    //    GL_TEXTURE18,
    //    GL_TEXTURE19,
    //    GL_TEXTURE20,
    //    GL_TEXTURE21,
    //    GL_TEXTURE22,
    //    GL_TEXTURE23,
    //    GL_TEXTURE24,
    //    GL_TEXTURE25,
    //    GL_TEXTURE26,
    //    GL_TEXTURE27,
    //    GL_TEXTURE28,
    //    GL_TEXTURE29,
    //    // NOTE KI 30 == depthMap
    //    GL_TEXTURE30,
    //    // NOTE KI 31 == skybox
    //    GL_TEXTURE31,
    //};

    static GLuint unitBase = FIRST_TEXTURE_UNIT;

    static GLuint indexBase = 0;
}

GLuint Texture::nextIndex()
{
    assert(indexBase < MAX_TEXTURE_COUNT - 1);
    return indexBase++;
}

GLuint Texture::getUnitIndexBase(int textureCount)
{
    GLuint idx = unitBase;

    if (idx + textureCount - 1 > LAST_TEXTURE_UNIT) {
        idx = FIRST_TEXTURE_UNIT;
        unitBase = FIRST_TEXTURE_UNIT;
    }
    else {
        unitBase += textureCount;
    }

    return idx;
}

GLuint Texture::nextUnitIndex()
{
    if (unitBase > LAST_TEXTURE_UNIT)
        unitBase = FIRST_TEXTURE_UNIT;
    return unitBase++ ;
}

Texture::Texture(const std::string& name, const TextureSpec& spec)
    : m_name(name),
    m_spec(spec)
{
}

Texture::~Texture()
{
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
    }
    if (m_handle != 0) {
        glMakeImageHandleNonResidentARB(m_handle);
    }
}
