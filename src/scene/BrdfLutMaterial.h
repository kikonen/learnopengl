#pragma once

#include <string>
#include <vector>
#include <array>

#include "material/CustomMaterial.h"

#include "render/BrdfLutTexture.h"

namespace kigl
{
    class GLState;
    class GLTextureHandle;
}

class BrdfLutMaterial : public CustomMaterial
{
public:
    BrdfLutMaterial();

    ~BrdfLutMaterial() = default;

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    virtual void bindTextures(kigl::GLState& state) override;

    const kigl::GLTextureHandle& getBrdfLutTextureHandle() const;

private:
    render::BrdfLutTexture m_brdfLutTexture;
};
