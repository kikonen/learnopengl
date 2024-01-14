#include "DynamicCubeMap.h"

#include "glm/ext.hpp"

#include "render/CubeMap.h"

#include "render/RenderContext.h"
#include "render/Batch.h"


DynamicCubeMap::DynamicCubeMap(
    std::string_view name,
    int size)
    : m_name(name),
    m_size(size),
    m_cubeMap{ m_name + "_dyn_cubemap_cube", true }
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

void DynamicCubeMap::prepareRT(
    const PrepareContext& ctx,
    const bool clear,
    const glm::vec4& clearColor)
{
    if (m_prepared) return;
    m_prepared = true;

    m_fbo.create(m_name + "_dyn_cubemap_fbo");

    m_cubeMap.m_size = m_size;
    m_cubeMap.m_internalFormat = GL_RGB16F;
    m_cubeMap.prepareRT(ctx);

    m_valid = true;
}

render::CubeMapBuffer DynamicCubeMap::asFrameBuffer(int face)
{
    return {
        "<dyn_cube_map_fbo>",
        static_cast<GLuint>(m_fbo),
        m_size,
        face,
        static_cast<GLuint>(m_cubeMap),
    };
}
