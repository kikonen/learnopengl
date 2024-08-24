#pragma once

#include "material/Material.h"

class Program;
class RenderContext;
struct PrepareContext;

namespace render {
    class FrameBuffer;
}

class PhysicsRenderer
{
public:
    void prepareRT(const PrepareContext& ctx);

    void render(
        const RenderContext& ctx,
        render::FrameBuffer* fbo);

private:
    void drawObjects(
        const RenderContext& ctx,
        render::FrameBuffer* targetBuffer);

private:
    Material m_fallbackMaterial;

    Program* m_objectProgram;

    uint32_t m_entityIndex{ 0 };
};
