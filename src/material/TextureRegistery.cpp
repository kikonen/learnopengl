#include "TextureRegistry.h"

#include "asset/Assets.h"

#include "util/util.h"

#include "kigl/kigl.h"

#include "material/ImageTexture.h"

namespace
{
    static TextureRegistry* s_registry{ nullptr };

    TextureSamplerType getSamplerType(const Texture* texture)
    {
        switch (texture->m_type) {
        case material::TextureType::map_normal:
            return TextureSamplerType::map_normal;
        case material::TextureType::map_displacement:
            return TextureSamplerType::map_displacement;
        case material::TextureType::map_height:
            return TextureSamplerType::map_height;
        }
        return TextureSamplerType::none;
    }
    //none,
    //    color_rgba,
    //    color_r,
    //    map_normal,
    //    map_displacement,
    //    map_height
}

void TextureRegistry::init() noexcept
{
    assert(!s_registry);
    s_registry = new TextureRegistry();
}

void TextureRegistry::release() noexcept
{
    auto* s = s_registry;
    s_registry = nullptr;
    delete s;
}

TextureRegistry& TextureRegistry::get() noexcept
{
    assert(s_registry);
    return *s_registry;
}

TextureRegistry::TextureRegistry()
{
    clear();
}

TextureRegistry::~TextureRegistry()
{
    clear();

    // TODO KI reserve NULL, black, white textures
    registerTexture({ nullptr });
    registerTexture({ nullptr });
    registerTexture({ nullptr });
}

void TextureRegistry::clear()
{
    m_typeTextures.clear();
}

uint64_t TextureRegistry::registerTexture(
    const Texture* texture)
{
    if (!texture) return 0;

    auto samplerType = getSamplerType(texture);

    auto& textures = m_typeTextures[samplerType];
    uint64_t index = static_cast<uint32_t>(textures.size());
    //textures.push_back(texture);

    auto handle = glGetTextureHandleARB(texture->m_textureID);
    glMakeTextureHandleResidentARB(handle);

    return handle;
}

void TextureRegistry::prepareRT()
{
    upload();
}

void TextureRegistry::updateRT()
{
    upload();
}

void TextureRegistry::upload()
{
    for (const auto& [type, textures] : m_typeTextures) {
        uint32_t size = static_cast<uint32_t>(textures.size());
        auto uploadedSize = m_typeUploadedSizes[type];

        if (size == uploadedSize) continue;

        for (uint32_t textureIndex = 0; textureIndex < size; textureIndex++) {
            const auto& texture = textures[textureIndex];
        }

        m_typeUploadedSizes[type] = size;
    }
}
