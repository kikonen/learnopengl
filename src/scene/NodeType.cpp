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

const std::string NodeType::str() const
{
    return fmt::format("<NODE_TYPE: id={}, mesh={}>", typeID, m_mesh ? m_mesh->str() : "N/A");
}


bool NodeType::hasReflection()
{
    if (!m_mesh) return false;
    return m_mesh->hasReflection();
}

bool NodeType::hasRefraction()
{
    if (!m_mesh) return false;
    return m_mesh->hasRefraction();
}

Material* NodeType::findMaterial(std::function<bool(const Material&)> fn)
{
    if (!m_mesh) return nullptr;
    return m_mesh->findMaterial(fn);
}

void NodeType::modifyMaterials(std::function<void(Material&)> fn)
{
    if (!m_mesh) return;
    m_mesh->modifyMaterials(fn);
}

void NodeType::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    if (!m_mesh) return;
    m_mesh->prepare(assets);

    Shader* shader = m_nodeShader;
    if (shader) {
        shader->prepare(assets);

        if (false) {
            ShaderBind bound(shader);
            shader->noiseTex.set(assets.noiseUnitIndex);
            shader->reflectionTex.set(assets.waterReflectionMapUnitIndex);
            shader->refractionTex.set(assets.waterRefractionMapUnitIndex);

            shader->cubeMap.set(assets.cubeMapUnitIndex);
            shader->shadowMap.set(assets.shadowMapUnitIndex);
            shader->skybox.set(assets.skyboxUnitIndex);
        }
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
    Shader* shader)
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
}

void NodeType::unbind(const RenderContext& ctx)
{
    m_boundShader = nullptr;
    ctx.bindGlobal();
}
