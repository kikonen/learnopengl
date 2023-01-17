#include "ModelRegistry.h"

#include <fmt/format.h>

#include "scene/RenderContext.h"
#include "asset/ModelMesh.h"
#include "asset/ModelMeshVBO.h"
#include "asset/MeshLoader.h"

#include "scene/Batch.h"

namespace {
}

ModelRegistry::ModelRegistry(const Assets& assets)
    : assets(assets)
{
}

ModelRegistry::~ModelRegistry() {
}

GLVertexArray* ModelRegistry::registerMeshVBO(ModelMeshVBO& meshVBO)
{
    //auto& vao = meshVBO.m_single ? m_singleVAO : m_multiVAO;
    //return vao.registerModel(meshVBO);
    //
    // NOTE KI multi/single material *CAN* go in same indirect draw
    return m_singleVAO.registerModel(meshVBO);
}

void ModelRegistry::prepare(Batch& batch)
{
    m_singleVAO.prepare(batch);
    m_multiVAO.prepare(batch);
}

std::shared_future<ModelMesh*> ModelRegistry::getMesh(
    const std::string& meshName)
{
    return getMesh(meshName, "");
}

std::shared_future<ModelMesh*> ModelRegistry::getMesh(
    const std::string& meshName,
    const std::string& meshPath)
{
    std::lock_guard<std::mutex> lock(m_meshes_lock);

    const std::string key = meshPath + "/" + meshName;

    {
        auto e = m_meshes.find(key);
        if (e != m_meshes.end())
            return e->second;
    }

    auto mesh = new ModelMesh(
        meshName,
        meshPath);

    auto future = startLoad(mesh);
    m_meshes[key] = future;

    return future;
}

std::shared_future<ModelMesh*> ModelRegistry::startLoad(ModelMesh* mesh)
{
    std::promise<ModelMesh*> promise;
    auto future = promise.get_future().share();

    // NOTE KI use thread instead of std::async since std::future blocking/cleanup is problematic
    // https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
    auto th = std::thread{
        [this, mesh, p = std::move(promise)]() mutable {
            try {
                KI_DEBUG(fmt::format("START_LOADER: {}", mesh->str()));

                MeshLoader loader(assets);
                auto loaded = loader.load(*mesh, m_defaultMaterial.get(), m_forceDefaultMaterial);

                if (loaded) {
                    loaded->prepareVolume();
                }

                p.set_value(mesh);
             } catch (const std::exception& ex) {
                KI_CRITICAL(ex.what());
                p.set_exception(std::make_exception_ptr(ex));
            } catch (...) {
                p.set_exception(std::make_exception_ptr(std::current_exception()));
            }
        }
    };
    th.detach();

    return future;
}
