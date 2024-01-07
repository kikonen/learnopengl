#pragma once

#include <string>

#include "kigl/GLTextureHandle.h"

struct PrepareContext;

class HdriTexture {
public:
    bool valid() { return m_texture.valid(); }

    void prepareRT(
        const PrepareContext& ctx);

    operator int() const { return m_texture; }

public:
    std::string m_path;

    kigl::GLTextureHandle m_texture;
};
