#pragma once
#include <functional>

#include <glm/glm.hpp>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "asset/Shader.h"



class RenderContext;
class FrameBuffer;

class Viewport final
{
public:
    Viewport(
        const std::string_view& name,
        const glm::vec3& pos,
        const glm::vec3& rotation,
        const glm::vec2& size,
        bool useFrameBuffer,
        unsigned int textureId,
        Shader* shader,
        std::function<void(Viewport&)> binder = [](Viewport&) {});

    ~Viewport();

    void setSourceFrameBuffer(FrameBuffer* frameBuffer);
    void setDestinationFrameBuffer(FrameBuffer* frameBuffer);

    void setTextureId(GLuint textureId);

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

    const bool m_useFrameBuffer;

    ViewportEffect m_effect = ViewportEffect::none;

private:
    bool m_prepared = false;

    GLVertexArray m_vao;
    GLBuffer m_vbo;

    FrameBuffer* m_sourceBuffer{ nullptr };
    FrameBuffer* m_destinationBuffer{ nullptr };

    GLuint m_textureId;

    Shader* m_shader{ nullptr };
    std::function<void(Viewport&)> m_binder;
};

