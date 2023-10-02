#pragma once
#include <functional>

#include <glm/glm.hpp>

#include "asset/Assets.h"

#include "kigl/GLBuffer.h"


class Program;

class UpdateContext;
class RenderContext;

class FrameBuffer;

class Viewport final
{
public:
    Viewport(
        std::string_view name,
        const glm::vec3& pos,
        const glm::vec3& rotation,
        const glm::vec2& size,
        bool m_useDirectBlit,
        unsigned int textureId,
        Program* program);

    ~Viewport();

    // NOTE KI called *before* binding program, to setup texture and such
    void setBindBefore(std::function<void(Viewport&)> binder) {
        m_bindBefore = binder;
    }

    // NOTE KI called *after* binding program, to setup uniforms and such
    void setBindAfter(std::function<void(Viewport&)> binder) {
        m_bindAfter = binder;
    }

    void setSourceFrameBuffer(FrameBuffer* frameBuffer);
    void setDestinationFrameBuffer(FrameBuffer* frameBuffer);

    void setTextureId(GLuint textureId);

    void prepare(const Assets& assets);

    void update(const UpdateContext& ctx);

    void bind(const RenderContext& ctx);
    void unbind(const RenderContext& ctx);
    void draw(const RenderContext& ctx);

    bool isEffectEnabled() {
        return m_effectEnabled;
    }

    void setEffectEnabled(bool enabled) {
        m_effectEnabled = enabled;
    }

    ViewportEffect getEffect() {
        return m_effect;
    }

    void setEffect(ViewportEffect effect) {
        m_effect = effect;
    }

    Program* getProgram() const noexcept {
        return m_program;
    }

private:
    void prepareTransform();

public:
    const std::string m_name;

    const glm::vec3 m_position;
    const glm::vec3 m_rotation;
    const glm::vec2 m_size;

    const bool m_useDirectBlit;

private:
    bool m_prepared = false;

    glm::mat4 m_transformMatrix{ 1.f };

    FrameBuffer* m_sourceBuffer{ nullptr };
    FrameBuffer* m_destinationBuffer{ nullptr };

    GLuint m_textureId;

    Program* m_program{ nullptr };

    bool m_effectEnabled{ false };
    ViewportEffect m_effect = ViewportEffect::none;

    std::function<void(Viewport&)> m_bindBefore{ [](Viewport&) {} };
    std::function<void(Viewport&)> m_bindAfter{ [](Viewport&) {} };
};
