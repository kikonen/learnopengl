#include "TextureRegistry.h"

#include "asset/Assets.h"

#include "util/util.h"

#include "kigl/kigl.h"

#include "material/ArrayTexture.h"
#include "material/ColorTexture.h"
#include "material/Texture.h"

#include "shader/Uniform.h"

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

    addArrayTexture({});

    // TODO KI reserve NULL, black, white textures
    registerTexture({ nullptr });
    registerTexture({ nullptr });
    registerTexture({ nullptr });
}

void TextureRegistry::clear()
{
    m_typeTextures.clear();
}

void TextureRegistry::prepareRT()
{
}

void TextureRegistry::updateRT()
{
    upload();
}

void TextureRegistry::bindBuffers()
{
    for (const auto& arr : m_arrayTextures) {
        if (auto uniform = arr->getUniformId(); uniform > 0) {
            glBindTextureUnit(uniform, arr->getTextureID());
        }
    }
}

// @return array ID
uint32_t TextureRegistry::addArrayTexture(const material::ArrayTextureInfo& info)
{
    uint32_t index = static_cast<uint32_t>(m_arrayTextures.size());

    const auto arr = util::Ref<ArrayTexture>::create(
        info.name,
        info.uniformId,
        info.grayScale,
        info.gammaCorrect,
        info.channels,
        info.is16Bit,
        info.size,
        info.size,
        info.maxLayers,
        info.hdri,
        info.spec);

    m_arrayTextures.push_back(arr);

    arr->prepareSingle();

    {
        int layer = arr->allocateLayer();
        assert(layer == material::TEX_ARRAY_LAYER_BLACK);
        const auto& px = ColorTexture::getBlackRGBA(false);
        px->prepareArray(arr, material::TEX_ARRAY_LAYER_BLACK);
    }
    {
        int layer = arr->allocateLayer();
        assert(layer == material::TEX_ARRAY_LAYER_WHITE);
        const auto& px = ColorTexture::getWhiteRGBA(false);
        px->prepareArray(arr, material::TEX_ARRAY_LAYER_WHITE);
    }
    {
        int layer = arr->allocateLayer();
        assert(layer == material::TEX_ARRAY_LAYER_NORMAL);
        const auto& px = ColorTexture::getFlatNormalRGBA(false);
        px->prepareArray(arr, material::TEX_ARRAY_LAYER_NORMAL);
    }

    arr->prepareMipMaps();

    return index;
}

void TextureRegistry::bindTextureType(material::TextureType type, uint32_t arrayId)
{
    m_mapping.insert({ type, arrayId });
}

uint64_t TextureRegistry::registerTexture(
    const util::Ref<Texture>& texture)
{
    if (!texture) return 0;

    const auto& assets = Assets::get();

    if (assets.drawUseArrayTexture) {
        const auto& it = m_mapping.find(texture->m_type);
        if (it == m_mapping.end()) return 0;

        util::Ref<ArrayTexture> arr = m_arrayTextures[it->second];
        uint32_t layer = arr->allocateLayer();
        texture->prepareArray(arr, layer);
        arr->prepareMipMaps();

        return layer;
    }
    else {
        texture->prepareSingle();
        texture->prepareHandle();
        return texture->m_handle;
    }
}

void TextureRegistry::upload()
{
    //for (const auto& [type, textures] : m_typeTextures) {
    //    uint32_t size = static_cast<uint32_t>(textures.size());
    //    auto uploadedSize = m_typeUploadedSizes[type];

    //    if (size == uploadedSize) continue;

    //    for (uint32_t textureIndex = 0; textureIndex < size; textureIndex++) {
    //        const auto& texture = textures[textureIndex];
    //    }

    //    m_typeUploadedSizes[type] = size;
    //}
}
