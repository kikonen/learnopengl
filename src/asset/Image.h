#pragma once

#include <string>


class Image final
{
public:
    Image(const std::string& path);
    ~Image();

    int load(bool flip);

public:
    const std::string m_path;

    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;

    bool m_flipped = false;
    unsigned char* m_data{ nullptr };

private:
    bool m_loaded = false;
    int m_res = 0;
};

