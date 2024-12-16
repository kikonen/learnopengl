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

    void setGammaCorrect(bool gammaCorrect) {
        m_gammaCorrect = gammaCorrect;
    }

    bool getGammaCorrect() const
    {
        return m_gammaCorrect;
    }

    void setHardwareGamma(bool hardwareGamma)
    {
        m_hardwareGamma = hardwareGamma;
    }

    bool getHardwareGamma() const
    {
        return m_hardwareGamma;
    }

private:
    std::unique_ptr<render::FrameBuffer> m_buffer{ nullptr };
    int m_width{ -1 };
    int m_height{ -1 };

    ki::program_id m_blitterId;

    bool m_gammaCorrect{ false };
    bool m_hardwareGamma{ false };
};
