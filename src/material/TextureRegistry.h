#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <future>

#include "util/Ref.h"

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

    uint64_t registerTexture(
        const Texture* texture);

private:
    void upload();

private:
    std::unordered_map<TextureSamplerType, std::vector<util::Ref<Texture>>> m_typeTextures;
    std::unordered_map<TextureSamplerType, uint32_t> m_typeUploadedSizes;
};

