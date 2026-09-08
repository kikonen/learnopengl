#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "util/Ref.h"

#include "ArrayTextureInfo.h"
#include "TextureType.h"

class ArrayTexture;
class Texture;

namespace material
{
    const inline int TEX_ARRAY_LAYER_NULL = 0;
    const inline int TEX_ARRAY_LAYER_BLACK = 1;
    const inline int TEX_ARRAY_LAYER_WHITE = 2;
    const inline int TEX_ARRAY_LAYER_NORMAL = 3;
}

class TextureRegistry final
{
public:
    static void init() noexcept;
    static void release() noexcept;
    static TextureRegistry& get() noexcept;

    TextureRegistry();
    TextureRegistry& operator=(const TextureRegistry&) = delete;

    ~TextureRegistry();

    void clear();

    void prepareRT();
    void updateRT();

    void bindBuffers();

    const util::Ref<ArrayTexture>& findArrayTexture(
        material::TextureType type);

    // @return array ID
    uint32_t addArrayTexture(const material::ArrayTextureInfo& info);

    void bindTextureType(material::TextureType type, uint32_t arrayId);

    uint64_t registerTexture(
        const util::Ref<Texture>& texture);

    void updateTexture(
        const util::Ref<Texture>& texture);

private:
    std::vector<util::Ref<ArrayTexture>> m_arrayTextures;
    std::unordered_map<material::TextureType, uint32_t> m_mapping;
};

