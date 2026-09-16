#include "TextureRegistry.h"

#include "asset/Assets.h"

#include "util/Log.h"
#include "util/util.h"

#include "kigl/kigl.h"
#include "kigl/GLState.h"

#include "material/ArrayTexture.h"
#include "material/ColorTexture.h"
#include "material/Texture.h"

#include "shader/Uniform.h"

namespace
{
    static TextureRegistry* s_registry{ nullptr };

    const util::Ref<ArrayTexture> NULL_TEXTURE;

    util::Ref<ColorTexture> getPixel(material::PixelType type)
    {
        switch (type) {
        case material::PixelType::none:
                return nullptr;
        case material::PixelType::magenta:
            return util::Ref<ColorTexture>::create(
                "MAGENTA_RGBA",
                glm::vec4{ 1.f, 0.f, 1.f, 1.f },
                GL_RGBA8);
        case material::PixelType::black:
            return util::Ref<ColorTexture>::create(
                "BLACK_RGBA",
                glm::vec4{ 0.f },
                GL_RGBA8);
        case material::PixelType::white:
            return util::Ref<ColorTexture>::create(
                "WHITE_RGBA",
                glm::vec4{ 1.f, 1.f, 1.f, 1.f },
                GL_RGBA8);
        case material::PixelType::normal:
            return util::Ref<ColorTexture>::create(
                "FLAT_NORMAL_RGBA",
                glm::vec4{ 0.5f, 0.5f, 1.f, 1.f },
                GL_RGBA8);
        }
        return nullptr;
    }
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
}

void TextureRegistry::clear()
{
    auto& state = kigl::GLState::get();

    for (const auto& arr : m_arrayTextures) {
        if (auto unitIndex = arr->getUnitIndex(); unitIndex > 0) {
            state.bindTexture(unitIndex, 0, true);
        }
    }

    m_arrayTextures.clear();
    m_mapping.clear();

    // NOTE KI reserve null texture
    addArrayTexture({});
}

void TextureRegistry::prepareRT()
{
}

void TextureRegistry::updateRT()
{
}

void TextureRegistry::bindTextures()
{
    auto& state = kigl::GLState::get();

    for (const auto& arr : m_arrayTextures) {
        if (auto unitIndex = arr->getUnitIndex(); unitIndex > 0) {
            state.bindTexture(unitIndex, arr->getTextureID(), false);
        }
    }
}
const util::Ref<ArrayTexture>& TextureRegistry::findArrayTexture(
    material::TextureType type)
{
    const auto& it = m_mapping.find(type);
    if (it == m_mapping.end())
        return NULL_TEXTURE;

    return m_arrayTextures[it->second];
}

// @return array ID
uint32_t TextureRegistry::addArrayTexture(const material::ArrayTextureInfo& info)
{
    const auto& assets = Assets::get();
    if (!assets.drawUseArrayTexture) return 0;

    uint32_t index = static_cast<uint32_t>(m_arrayTextures.size());

    const auto arr = util::Ref<ArrayTexture>::create(
        info.type,
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

    if (arr->getUnitIndex() != 0) {
        arr->prepareSingle();

        // NULL
        if (false) {
            const auto& px = getPixel(material::PixelType::magenta);
            if (px) {
                arr->registerTexture(px);
            }
        }
        if (true) {
            arr->allocateLayer();
        }

        if (true) {
            for (const auto& pixelType : info.pixels) {
                const auto& px = getPixel(pixelType);
                if (px) {
                    arr->registerTexture(px);
                }
            }
        }

        if (false) {
            {
                int layer = arr->peekNextLayerIndex();
                assert(layer == material::TEX_ARRAY_LAYER_BLACK);
                const auto& px = util::Ref<ColorTexture>::create(
                    "BLACK_RGBA",
                    glm::vec4{ 0.f },
                    GL_RGBA8);
                arr->registerTexture(px);
            }
            {
                int layer = arr->peekNextLayerIndex();
                assert(layer == material::TEX_ARRAY_LAYER_WHITE);
                const auto& px = util::Ref<ColorTexture>::create(
                    "WHITE_RGBA",
                    glm::vec4{ 1.f, 1.f, 1.f, 1.f },
                    GL_RGBA8);
                arr->registerTexture(px);
            }
            {
                int layer = arr->peekNextLayerIndex();
                assert(layer == material::TEX_ARRAY_LAYER_NORMAL);
                const auto& px = util::Ref<ColorTexture>::create(
                    "FLAT_NORMAL_RGBA",
                    glm::vec4{ 0.5f, 0.5f, 1.f, 1.f },
                    GL_RGBA8);
                arr->registerTexture(px);
            }
        }

        arr->updateMipMaps();
    }

    return index;
}

void TextureRegistry::bindTextureType(material::TextureType type, uint32_t arrayId)
{
    m_mapping.insert({ type, arrayId });
}

uint32_t TextureRegistry::registerTexture(
    const util::Ref<Texture>& texture)
{
    if (!texture) {
        KI_WARN_OUT("TEX::REGISTRY: attempt to register null texture");
        return 0;
    }

    if (m_arrayTextures.empty()) {
        KI_CRITICAL_OUT("TEX::REGISTRY: empty registry");
        return 0;
    }

    const auto& assets = Assets::get();

    const auto& it = m_mapping.find(texture->m_type);
    if (it == m_mapping.end())
        return 0;

    util::Ref<ArrayTexture> arr = m_arrayTextures[it->second];
    uint32_t layer = arr->registerTexture(texture);
    arr->updateMipMaps();

    return layer;
}

void TextureRegistry::updateTexture(
    const util::Ref<Texture>& texture)
{
    if (!texture) return;
    // NOTE KI don't update textures which are not registered
    if (texture->getLayer() == 0) return;

    const auto& assets = Assets::get();

    const auto& it = m_mapping.find(texture->m_type);
    if (it == m_mapping.end())
        return;

    util::Ref<ArrayTexture> arr = m_arrayTextures[it->second];
    arr->updateTexture(texture);
    arr->updateMipMaps();
}
