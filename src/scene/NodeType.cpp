#include "NodeType.h"

#include <mutex>
#include <fmt/format.h>

#include "asset/Assets.h"
#include "asset/ShaderBind.h"

#include "RenderContext.h"

namespace {
    int typeIDbase = 0;

    std::mutex type_id_lock;

    int nextID()
    {
        std::lock_guard<std::mutex> lock(type_id_lock);
        return ++typeIDbase;
    }
}


NodeType::NodeType()
    : typeID(nextID())
{
}

NodeType::~NodeType()
{
    KI_INFO_SB("NODE_TYPE: delete " << typeID);
}

const std::string NodeType::str() const noexcept
{
    return fmt::format("<NODE_TYPE: id={}, mesh={}>", typeID, m_mesh ? m_mesh->str() : "N/A");
}


bool NodeType::hasReflection() noexcept
{
    if (!m_mesh) return false;
    return m_mesh->hasReflection();
}

bool NodeType::hasRefraction() noexcept
{
    if (!m_mesh) return false;
    return m_mesh->hasRefraction();
}

Material* NodeType::findMaterial(std::function<bool(const Material&)> fn) noexcept
{
    if (!m_mesh) return nullptr;
    return m_mesh->findMaterial(fn);
}

void NodeType::modifyMaterials(std::function<void(Material&)> fn) noexcept
{
    if (!m_mesh) return;
    m_mesh->modifyMaterials(fn);
}

void NodeType::prepare(const Assets& assets) noexcept
{
    if (m_prepared) return;
    m_prepared = true;

    if (!m_mesh) return;
    m_mesh->prepare(assets);

    Shader* shader = m_nodeShader;
    if (shader) {
        shader->prepare(assets);
    }

    if (m_batch.batchSize < 0) {
        m_batch.batchSize = assets.batchSize;
    }
    if (m_flags.instanced) {
        m_batch.batchSize = 0;
    }

    if (m_batch.batchSize > 0)
        m_batch.prepare(*this);
}

void NodeType::bind(
    const RenderContext& ctx, 
    Shader* shader) noexcept
{
    if (!m_mesh) return;

    m_boundShader = shader;

    // NOTE KI material needed for alpha shadows
    bool bindMaterials = !ctx.shadow || m_flags.alpha;
    m_mesh->bind(ctx, shader, bindMaterials);

    if (m_flags.renderBack) {
        ctx.state.disable(GL_CULL_FACE);
    }
    else {
        ctx.state.enable(GL_CULL_FACE);
    }

    if (m_flags.wireframe) {
        ctx.state.polygonFrontAndBack(GL_LINE);
    }

    // NOTE KI reflection map varies depending of the rendered entity
    if (m_flags.water) {
        m_boundShader->noiseTex.set(ctx.assets.noiseUnitIndex);
        m_boundShader->reflectionTex.set(ctx.assets.waterReflectionMapUnitIndex);
        m_boundShader->refractionTex.set(ctx.assets.waterRefractionMapUnitIndex);
    } else if (m_flags.mirror) {
        m_boundShader->reflectionTex.set(ctx.assets.mirrorReflectionMapUnitIndex);
    }
}

void NodeType::unbind(const RenderContext& ctx) noexcept
{
    m_boundShader = nullptr;
    ctx.bindGlobal();
}
