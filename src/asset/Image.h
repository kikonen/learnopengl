#pragma once

#include <string>


class Image final
{
public:
    Image(std::string_view path);

    Image(
        std::string_view path,
        bool flipped);

    Image(
        std::string_view path,
        bool flipped,
        bool hdri);

    ~Image();

    int load();

public:
    const std::string m_path;
    const bool m_flipped;
    const bool m_hdri;

    // NOTE KI stbi_load requires int
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
    bool m_is16Bbit = false;

    unsigned char* m_data{ nullptr };

private:
    bool m_loaded = false;
    int m_res = 0;
};

