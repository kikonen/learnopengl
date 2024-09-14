#pragma once
#include <functional>

#include <glm/glm.hpp>

#include "asset/ViewportEffect.h"

#include "kigl/GLBuffer.h"

namespace render {
    class FrameBuffer;
}

class Program;
struct UpdateViewContext;
class RenderContext;

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
        ki::program_id programId);

    ~Viewport();

    // NOTE KI called from update cycle
    void setUpdate(std::function<void(Viewport&, const UpdateViewContext& ctx)> update) {
        m_update = update;
    }

    // NOTE KI called *before* binding program, to setup texture and such
    void setBindBefore(std::function<void(Viewport&)> binder) {
        m_bindBefore = binder;
    }

    // NOTE KI called *after* binding program, to setup uniforms and such
    void setBindAfter(std::function<void(Viewport&)> binder) {
        m_bindAfter = binder;
    }

    void invokeBindBefore();
    void invokeBindAfter();

    render::FrameBuffer* getSourceFrameBuffer() const
    {
        return m_sourceBuffer;
    }

    void setSourceFrameBuffer(render::FrameBuffer* frameBuffer);
    void setDestinationFrameBuffer(render::FrameBuffer* frameBuffer);

    void setTextureId(GLuint textureId);

    void prepareRT();

    void updateRT(const UpdateViewContext& ctx);

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

    void setPosition(const glm::vec3& pos) {
        if (m_position != pos) {
            m_position = pos;
            m_dirty = true;
        }
    }

    const glm::vec3& getPosition() {
        return m_position;
    }

    void setDegreesRotation(const glm::vec3& euler) {
        if (m_degreesRotation != euler) {
            m_degreesRotation = euler;
            m_dirty = true;
        }
    }

    const glm::vec3& getDegreesRotation() {
        return m_degreesRotation;
    }

    void setSize(const glm::vec2& size) {
        if (m_size != size) {
            m_size = size;
            m_dirty = true;
        }
    }

    const glm::vec2& getSize() {
        return m_size;
    }

    void setGammaCorrect(bool gammaCorrect) {
        m_gammaCorrect = gammaCorrect;
    }

    bool getGammaCorrect() {
        return m_gammaCorrect;
    }

    void setHardwareGamma(bool hardwareGamma)
    {
        m_hardwareGamma = hardwareGamma;
    }

    bool getHardwareGamma()
    {
        return m_hardwareGamma;
    }

private:
    void prepareTransform();
    void updateTransform(const UpdateViewContext& ctx);

public:
    const std::string m_name;

    const bool m_useDirectBlit;

private:
    bool m_prepared = false;

    glm::vec3 m_position;
    glm::vec3 m_degreesRotation;
    glm::vec2 m_size;

    glm::mat4 m_projected;

    bool m_gammaCorrect{ false };
    bool m_hardwareGamma{ false };

    bool m_dirty{ true };

    glm::mat4 m_transformMatrix{ 1.f };

    render::FrameBuffer* m_sourceBuffer{ nullptr };
    render::FrameBuffer* m_destinationBuffer{ nullptr };

    GLuint m_textureId;

    Program* m_program{ nullptr };

    bool m_effectEnabled{ false };
    ViewportEffect m_effect = ViewportEffect::none;

    std::function<void(Viewport&, const UpdateViewContext&)> m_update{ [](Viewport&, const UpdateViewContext& ctx) {} };
    std::function<void(Viewport&)> m_bindBefore{ [](Viewport&) {} };
    std::function<void(Viewport&)> m_bindAfter{ [](Viewport&) {} };
};
