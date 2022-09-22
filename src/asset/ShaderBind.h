#pragma once

#include "asset/Shader.h"

class ShaderBind final
{
public:
    ShaderBind(Shader* shader) : shader(shader) {
        if (shader) shader->bind();
    };

    ~ShaderBind() {
        if (shader) shader->unbind();
    }

public:
    Shader* shader;
};

