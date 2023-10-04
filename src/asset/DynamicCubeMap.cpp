#include "DynamicCubeMap.h"

#include "glm/ext.hpp"

#include "render/CubeMap.h"

#include "render/RenderContext.h"
#include "render/Batch.h"
#include "render/FrameBufferAttachment.h"


DynamicCubeMap::DynamicCubeMap(int size)
    : m_size(size)
{
}

DynamicCubeMap::~DynamicCubeMap()
{
}

void DynamicCubeMap::bindTexture(const RenderContext& ctx, int unitIndex)
{
    m_cubeMap.bindTexture(ctx, unitIndex);
}

void DynamicCubeMap::bind(const RenderContext& ctx)
{
    // NOTE KI must flush before changing render target
    //ctx.m_batch->flush(ctx);
    if (!ctx.m_batch->isFlushed()) {
        throw std::runtime_error{ fmt::format("BIND_ERROR: Batch was NOT flushed: FBO={}", (int)m_fbo) };
    }

    ctx.m_state.bindFrameBuffer(m_fbo, false);
    ctx.m_state.setViewport({ 0, 0, m_size, m_size });
}

void DynamicCubeMap::unbind(const RenderContext& ctx)
{
    //ctx.m_state.bindFrameBuffer(0, false);
}

void DynamicCubeMap::prepare(
    const Assets& assets,
    Registry* registry,
    const bool clear,
    const glm::vec4& clearColor)
{
    if (m_prepared) return;
    m_prepared = true;

    m_fbo.create("dynamic_cube_map");

    m_cubeMap.m_size = m_size;
    m_cubeMap.m_internalFormat = GL_RGB16F;
    m_cubeMap.prepare(assets, registry);

    m_valid = true;
}

CubeMapBuffer DynamicCubeMap::asFrameBuffer(int face)
{
    return {
        (GLuint)m_fbo,
        m_size,
        face,
        (GLuint)m_cubeMap,
    };
}
