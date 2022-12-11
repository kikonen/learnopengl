#pragma once
#include <functional>

#include <glm/glm.hpp>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "asset/Shader.h"

class RenderContext;

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

private:
    void prepareVBO();

public:
    const std::string m_name;

    const glm::vec3 m_pos;
    const glm::vec3 m_rotation;
    const glm::vec2 m_size;

    ViewportEffect m_effect = ViewportEffect::none;

private:
    bool m_prepared = false;

    GLVertexArray m_vao;
    GLBuffer m_vbo;

    unsigned int m_textureID;

    Shader* m_shader{ nullptr };
    std::function<void(Viewport&)> m_binder;
};

