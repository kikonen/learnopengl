#pragma once

#include <string>

#include "kigl/GLTextureHandle.h"

class Assets;
class Registry;


class HdriTexture {
public:
    bool valid() { return m_texture.valid(); }

    void prepareRT(
        const Assets& assets,
        Registry* registry);

    operator int() const { return m_texture; }

public:
    std::string m_path;

    kigl::GLTextureHandle m_texture;
};
