#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "kigl/kigl.h"

#include "ki/size.h"
#include "ki/sid.h"

#include "material/MaterialUpdater.h"

struct Material;

namespace render {
    class FrameBuffer;
}

class ShaderMaterialUpdater : public MaterialUpdater
{
public:
    ShaderMaterialUpdater(
        ki::StringID id,
        const std::string& name);

    ~ShaderMaterialUpdater();

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    virtual void render(
        const RenderContext& ctx) override;

    virtual GLuint64 getTexHandle(TextureType type) const noexcept override;

public:
    glm::ivec2 m_size;
    float m_updateSpeed{ 0.f };
    int m_frameSkip{ 1 };

private:
    std::unique_ptr<render::FrameBuffer> m_buffer{ nullptr };

    GLuint64 m_handle{ 0 };
    GLuint m_samplerId{ 0 };

    int m_frameCounter{ 0 };
};
