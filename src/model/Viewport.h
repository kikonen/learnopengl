#pragma once
#include <functional>

#include <glm/glm.hpp>

#include "asset/ViewportEffect.h"
#include "asset/LayerInfo.h"

#include "kigl/GLBuffer.h"

namespace render {
    class FrameBuffer;
}

namespace render
{
    class RenderContext;
}

class Program;
struct UpdateViewContext;

namespace model
{
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

        void setTextureId(GLuint textureId);

        void prepareRT();

        void updateRT(const UpdateViewContext& ctx);

        void bind(const render::RenderContext& ctx);

        void draw(
            const render::RenderContext& ctx,
            render::FrameBuffer* destinationBuffer);

        bool isEnabled() const
        {
            return m_enabled;
        }

        void setEnabled(bool enabled)
        {
            m_enabled = enabled;
        }

        bool isEffectEnabled() const
        {
            return m_effectEnabled;
        }

        void setEffectEnabled(bool enabled) {
            m_effectEnabled = enabled;
        }

        ViewportEffect getEffect() const {
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

        const glm::vec3& getPosition() const
        {
            return m_position;
        }

        void setDegreesRotation(const glm::vec3& euler) {
            if (m_degreesRotation != euler) {
                m_degreesRotation = euler;
                m_dirty = true;
            }
        }

        const glm::vec3& getDegreesRotation() const
        {
            return m_degreesRotation;
        }

        void setSize(const glm::vec2& size) {
            if (m_size != size) {
                m_size = size;
                m_dirty = true;
            }
        }

        void applyLayer(const LayerInfo& layer)
        {
            setIndex(layer.m_index);
            setOrder(layer.m_order);
            setEnabled(layer.m_enabled);
            setEffectEnabled(layer.m_effectEnabled);
            setEffect(layer.m_effect);
            setBlend(layer.m_blendEnabled);
            setBlendFactor(layer.m_blendFactor);
        }

        const glm::vec2& getSize() const
        {
            return m_size;
        }

        int getIndex() const
        {
            return m_index;
        }

        void setIndex(int index)
        {
            m_index = index;
        }

        int getOrder() const
        {
            return m_order;
        }

        void setOrder(int order)
        {
            m_order = order;
        }

        bool isBlend() const
        {
            return m_blend;
        }

        void setBlend(bool blend)
        {
            m_blend = blend;
        }

        float getBlendFactor() const
        {
            return m_blendFactor;
        }

        void setBlendFactor(float blendFactor)
        {
            m_blendFactor = blendFactor;
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

        int m_index{ -1 };
        int m_order{ 100 };
        bool m_blend{ true };
        float m_blendFactor{ 1.f };

        bool m_enabled{ true };

        bool m_dirty{ true };

        glm::mat4 m_transformMatrix{ 1.f };

        render::FrameBuffer* m_sourceBuffer{ nullptr };
        render::FrameBuffer* m_destinationBuffer{ nullptr };

        GLuint m_textureId;

        Program* m_program{ nullptr };

        bool m_effectEnabled{ false };
        ViewportEffect m_effect = ViewportEffect::none;

        std::function<void(model::Viewport&, const UpdateViewContext&)> m_update{ [](Viewport&, const UpdateViewContext& ctx) {} };
        std::function<void(model::Viewport&)> m_bindBefore{ [](Viewport&) {} };
        std::function<void(model::Viewport&)> m_bindAfter{ [](Viewport&) {} };
    };
}
