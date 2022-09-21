#pragma once
#include <functional>

#include "asset/MeshBuffers.h"

#include "scene/RenderContext.h"

enum class ViewportEffect {
    none = 0,
    invert = 1,
    grayScale = 2,
    sharpen = 3,
    blur = 4,
    edge = 5,
};


class Viewport final
{
public:
    Viewport(
        const glm::vec3& pos, 
        const glm::vec3& rotation, 
        const glm::vec2& size, 
        unsigned int textureID, 
        Shader* shader,
        std::function<void(Viewport&)> binder = [](Viewport&) {});

    ~Viewport();

    void setTextureID(unsigned int textureID);

    void prepare(const Assets& assets);

    void update(const RenderContext& ctx);
    void bind(const RenderContext& ctx);
    void unbind(const RenderContext& ctx);
    void draw(const RenderContext& ctx);

public:
    ViewportEffect effect = ViewportEffect::none;

    const glm::vec3 pos;
    const glm::vec3 rotation;
    const glm::vec2 size;

private:
    MeshBuffers buffers;

    unsigned int textureID;

    Shader* shader{ nullptr };
    std::function<void(Viewport&)> binder;
};

