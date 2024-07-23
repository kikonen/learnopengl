#pragma once

#include "asset/Material.h"

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
    Material m_objectMaterial;
    Program* m_objectProgram;

    uint32_t m_entityIndex{ 0 };
    uint32_t m_meshIndex{ 0 };
};
