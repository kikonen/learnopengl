#include "ModelRegistry.h"

#include <fmt/format.h>

#include "asset/Assets.h"

#include "mesh/MeshSet.h"

#include "mesh/vao/TexturedVAO.h"
#include "mesh/vao/SkinnedVAO.h"

#include "mesh/AssimpLoader.h"

#include "render/RenderContext.h"

namespace {
    static ModelRegistry s_registry;
}

ModelRegistry& ModelRegistry::get() noexcept
{
    return s_registry;
}

ModelRegistry::ModelRegistry()
    : m_texturedVao{ std::make_unique<mesh::TexturedVAO>("mesh_textured") },
    m_skinnedVao{ std::make_unique<mesh::SkinnedVAO>("mesh_skinned") }
{
}

ModelRegistry::~ModelRegistry() {
}

void ModelRegistry::prepare(std::shared_ptr<std::atomic<bool>> alive)
{
    m_alive = alive;
    m_texturedVao->prepare();
    m_skinnedVao->prepare();
}

void ModelRegistry::updateRT(const UpdateContext& ctx)
{
    m_texturedVao->updateRT();
    m_skinnedVao->updateRT();
}

std::shared_future<mesh::MeshSet*> ModelRegistry::getMeshSet(
    std::string_view rootDir,
    std::string_view meshPath)
{
    if (!*m_alive) return {};

    std::lock_guard lock(m_meshes_lock);

    // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
    std::string key = util::joinPath(
        rootDir,
        meshPath);

    {
        auto e = m_meshes.find(key);
        if (e != m_meshes.end())
            return e->second;
    }

    auto meshSet = new mesh::MeshSet(
        rootDir,
        meshPath);

    auto future = startLoad(meshSet);
    m_meshes[key] = future;

    return future;
}

std::shared_future<mesh::MeshSet*> ModelRegistry::startLoad(mesh::MeshSet* meshSet)
{
    std::promise<mesh::MeshSet*> promise;
    auto future = promise.get_future().share();

    // NOTE KI use thread instead of std::async since std::future blocking/cleanup is problematic
    // https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
    auto th = std::thread{
        [this, meshSet, p = std::move(promise)]() mutable {
            try {
                const auto assets = Assets::get();

                KI_DEBUG(fmt::format("START_LOADER: {}", meshSet->str()));

                std::unique_ptr<mesh::ModelLoader> loader;

                if (assets.useAssimpLoader) {
                    loader = std::make_unique<mesh::AssimpLoader>(m_alive);
                }
                else {
                    throw "no loader";
                }

                auto loaded = loader->load(*meshSet, m_defaultMaterial.get(), m_forceDefaultMaterial);

                // NOTE KI if not valid then null; avoids internal errors in render logic
                if (loaded) {
                    meshSet->prepareVolume();
                    p.set_value(meshSet);
                }
                else {
                    KI_CRITICAL(fmt::format("FAIL_LOADER: Invalid mesh: {}", meshSet->str()));
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
