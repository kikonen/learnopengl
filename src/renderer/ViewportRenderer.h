#pragma once

#include <memory>

#include "ki/size.h"

#include "Renderer.h"

namespace render {
    class FrameBuffer;
}

class ViewportRenderer final : public Renderer
{
public:
    ViewportRenderer(bool useFrameStep);
    ~ViewportRenderer();

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void updateRT(const UpdateViewContext& ctx);

    void render(
        const RenderContext& ctx,
        render::FrameBuffer* destinationBuffer);

    void setGammaCorrectEnabled(bool enabled) {
        m_gammaCorrectEnabled = enabled;
    }

    bool getGammaCorrectEnabled() const noexcept
    {
        return m_gammaCorrectEnabled;
    }

    void setHardwareGammaEnabled(bool enabled)
    {
        m_hardwareGammaEnabled = enabled;
    }

    bool getHardwareGammaEnabled() const noexcept
    {
        return m_hardwareGammaEnabled;
    }

    void setHdrToneMappingEnabled(bool enabled)
    {
        m_hdrToneMappingEnabled = enabled;
    }

    bool getHdrToneMappingEnabled() const noexcept
    {
        return m_hdrToneMappingEnabled;
    }

private:
    std::unique_ptr<render::FrameBuffer> m_buffer{ nullptr };
    int m_width{ -1 };
    int m_height{ -1 };

    ki::program_id m_blitterId;

    bool m_gammaCorrectEnabled{ false };
    bool m_hardwareGammaEnabled{ false };
    bool m_hdrToneMappingEnabled{ false };
};
