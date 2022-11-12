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
        const std::string_view& name,
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
    const std::string m_name;

    const glm::vec3 m_pos;
    const glm::vec3 m_rotation;
    const glm::vec2 m_size;

    ViewportEffect m_effect = ViewportEffect::none;

private:
    bool m_prepared = false;

    MeshBuffers m_buffers;

    unsigned int m_textureID;

    Shader* m_shader{ nullptr };
    std::function<void(Viewport&)> m_binder;
};

