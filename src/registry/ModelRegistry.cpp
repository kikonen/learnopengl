#include "ModelRegistry.h"

#include <fmt/format.h>

#include "mesh/ModelMesh.h"
#include "mesh/ModelVBO.h"
#include "mesh/ModelLoader.h"

#include "render/RenderContext.h"

namespace {
}

ModelRegistry::ModelRegistry(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_assets(assets),
    m_alive(alive)
{
}

ModelRegistry::~ModelRegistry() {
}

void ModelRegistry::prepare()
{
    m_vao.prepare("model");
}

kigl::GLVertexArray* ModelRegistry::registerModelVBO(mesh::ModelVBO& modelVBO)
{
    return m_vao.registerModel(modelVBO);
}

void ModelRegistry::updateRT(const UpdateContext& ctx)
{
    m_vao.updateRT();
}

std::shared_future<mesh::ModelMesh*> ModelRegistry::getMesh(
    std::string_view meshName,
    std::string_view rootDir)
{
    return getMesh(meshName, rootDir, "");
}

std::shared_future<mesh::ModelMesh*> ModelRegistry::getMesh(
    std::string_view meshName,
    std::string_view rootDir,
    std::string_view meshPath)
{
    if (!*m_alive) return {};

    std::lock_guard lock(m_meshes_lock);

    // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
    std::string key = util::joinPathExt(
        rootDir,
        meshPath,
        meshName, "");

    {
        auto e = m_meshes.find(key);
        if (e != m_meshes.end())
            return e->second;
    }

    auto mesh = new mesh::ModelMesh(
        meshName,
        rootDir,
        meshPath);

    auto future = startLoad(mesh);
    m_meshes[key] = future;

    return future;
}

std::shared_future<mesh::ModelMesh*> ModelRegistry::startLoad(mesh::ModelMesh* mesh)
{
    std::promise<mesh::ModelMesh*> promise;
    auto future = promise.get_future().share();

    // NOTE KI use thread instead of std::async since std::future blocking/cleanup is problematic
    // https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
    auto th = std::thread{
        [this, mesh, p = std::move(promise)]() mutable {
            try {
                std::string info = mesh->str();

                KI_DEBUG(fmt::format("START_LOADER: {}", info));

                mesh::ModelLoader loader(m_assets, m_alive);
                auto loaded = loader.load(*mesh, m_defaultMaterial.get(), m_forceDefaultMaterial);

                if (loaded) {
                    loaded->prepareVolume();
                }

                // NOTE KI if not valid then null; avoids internal errors in render logic
                if (loaded) {
                    p.set_value(mesh);
                }
                else {
                    KI_CRITICAL(fmt::format("FAIL_LOADER: Invalid mesh: {}", info));
                    p.set_value(nullptr);
                }
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
