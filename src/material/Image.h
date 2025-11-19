#pragma once

#include <string>
#include <vector>

class Image final
{
public:
    Image();

    Image(
        std::string_view path,
        bool flipped);

    Image(
        std::string_view path,
        bool flipped,
        bool hdri);

    Image(Image& o) = delete;
    Image& operator=(Image& o) = delete;
    Image& operator=(Image&& o) = delete;
    Image(Image&& o) noexcept;

    ~Image();

    int load();

    int loadNormal();
    int loadKtx();

    int loadFromMememory(std::vector<unsigned char> data);

public:
    const std::string m_path;
    const bool m_flipped;
    const bool m_hdri;
    const bool m_ktx;

    // NOTE KI stbi_load requires int
    int m_width{ 0 };
    int m_height{ 0 };
    int m_channels{ 0 };
    bool m_is16Bbit{ false };

    unsigned char* m_data{ nullptr };

private:
    bool m_loaded{ false };
    int m_res{ 0 };
};

