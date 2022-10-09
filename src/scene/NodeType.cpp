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
    return fmt::format("<NODE_TYPE: id={}, mesh={}>", typeID, mesh ? mesh->str() : "N/A");
}


bool NodeType::hasReflection()
{
    if (!mesh) return false;
    return mesh->hasReflection();
}

bool NodeType::hasRefraction()
{
    if (!mesh) return false;
    return mesh->hasRefraction();
}

Material* NodeType::findMaterial(std::function<bool(const Material&)> fn)
{
    if (!mesh) return nullptr;
    return mesh->findMaterial(fn);
}

void NodeType::modifyMaterials(std::function<void(Material&)> fn)
{
    if (!mesh) return;
    mesh->modifyMaterials(fn);
}

void NodeType::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    if (!mesh) return;
    mesh->prepare(assets);

    Shader* shader = nodeShader;
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

    if (batch.batchSize < 0) {
        batch.batchSize = assets.batchSize;
    }
    if (flags.instanced) {
        batch.batchSize = 0;
    }

    if (batch.batchSize > 0)
        batch.prepare(*this);
}

void NodeType::bind(
    const RenderContext& ctx, 
    Shader* shader)
{
    if (!mesh) return;

    boundShader = shader;

    mesh->bind(ctx, shader);

    if (flags.renderBack) {
        ctx.state.disable(GL_CULL_FACE);
    }
    else {
        ctx.state.enable(GL_CULL_FACE);
    }

    if (flags.wireframe) {
        ctx.state.polygonFrontAndBack(GL_LINE);
    }
}

void NodeType::unbind(const RenderContext& ctx)
{
    boundShader = nullptr;
    ctx.bindGlobal();
}
