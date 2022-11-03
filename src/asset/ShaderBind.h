#pragma once

#include "asset/Shader.h"

class ShaderBind final
{
public:
    ShaderBind(Shader* shader) noexcept
        : shader(shader)
    {
        if (shader) shader->bind();
    };

    ~ShaderBind() noexcept {
        if (shader) shader->unbind();
    }

public:
    Shader* shader;
};

