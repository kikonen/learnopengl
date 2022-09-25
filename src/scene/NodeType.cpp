#include "NodeType.h"

#include <mutex>

#include "asset/Assets.h"
#include "asset/ShaderBind.h"

#include "RenderContext.h"

namespace {
    int typeIDbase = 0;

    std::mutex type_id_lock;
}

int NodeType::nextID()
{
    std::lock_guard<std::mutex> lock(type_id_lock);
    return ++typeIDbase;
}

void NodeType::setBaseID(int baseId)
{
    std::lock_guard<std::mutex> lock(type_id_lock);
    typeIDbase = baseId;
}

NodeType::NodeType(int typeID)
    : typeID(typeID)
{
}

NodeType::~NodeType()
{
    KI_INFO_SB("NODE_TYPE: delete " << typeID);
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

    if (flags.batchMode && batch.batchSize > 0) {
        batch.prepare(this);
    }
    else {
        batch.batchSize = 0;
    }
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
