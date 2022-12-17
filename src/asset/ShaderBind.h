#pragma once

#include "kigl/GLState.h"

#include "asset/Shader.h"

class ShaderBind final
{
public:
    ShaderBind(Shader* shader, GLState& state) noexcept
        : shader(shader)
    {
        if (shader) shader->bind(state);
    };

    ~ShaderBind() noexcept {
        //if (shader) shader->unbind();
    }

public:
    Shader* shader;
};

