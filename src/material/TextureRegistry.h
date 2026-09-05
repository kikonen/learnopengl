#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "util/Ref.h"

#include "ArrayTextureInfo.h"
#include "TextureType.h"

class ArrayTexture;
class Texture;

// Mapped from TextureType into sampler2dArray typing
enum class TextureSamplerType : std::underlying_type_t<std::byte>
{
    none,
    color_rgba,
    color_r,
    map_normal,
    map_displacement,
    map_height
};

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

    // @return array ID
    uint32_t addArrayTexture(const material::ArrayTextureInfo& info);

    void bindTextureType(material::TextureType type, uint32_t arrayId);

    uint64_t registerTexture(
        const util::Ref<Texture>& texture);

private:
    void upload();

private:
    std::unordered_map<TextureSamplerType, std::vector<util::Ref<Texture>>> m_typeTextures;
    std::unordered_map<TextureSamplerType, uint32_t> m_typeUploadedSizes;

    std::vector<util::Ref<ArrayTexture>> m_arrayTextures;
    std::unordered_map<material::TextureType, uint32_t> m_mapping;
};

