#include "ModelRegistry.h"

#include <fmt/format.h>

#include "util/Log.h"

#include "asset/Assets.h"

#include "mesh/MeshSet.h"
#include "mesh/ModelMesh.h"

#include "mesh/vao/TexturedVAO.h"
#include "mesh/vao/SkinnedVAO.h"

#include "mesh/AssimpLoader.h"

#include "render/RenderContext.h"

namespace
{
    thread_local std::exception_ptr lastException = nullptr;

    static ModelRegistry* s_registry{ nullptr };
}

void ModelRegistry::init() noexcept
{
    assert(!s_registry);
    s_registry = new ModelRegistry();
}

void ModelRegistry::release() noexcept
{
    auto* s = s_registry;
    s_registry = nullptr;
    delete s;
}

ModelRegistry& ModelRegistry::get() noexcept
{
    assert(s_registry);
    return *s_registry;
}

ModelRegistry::ModelRegistry()
{
    clear();
}

ModelRegistry::~ModelRegistry() {
    clear();
}

void ModelRegistry::clear()
{
    m_meshes.clear();
}

void ModelRegistry::prepare(std::shared_ptr<std::atomic<bool>> alive)
{
    m_alive = alive;
}

std::shared_future<std::shared_ptr<mesh::MeshSet>> ModelRegistry::getMeshSet(
    std::string_view id,
    std::string_view rootDir,
    std::string_view meshPath,
    bool smoothNormals,
    bool forceNormals)
{
    if (!*m_alive) return {};

    std::lock_guard lock(m_meshes_lock);

    // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
    std::string key = fmt::format(
        "{}_{}_{}_{}_{}",
        id,
        rootDir,
        meshPath,
        smoothNormals,
        forceNormals);

    {
        auto e = m_meshes.find(key);
        if (e != m_meshes.end())
            return e->second;
    }

    auto meshSet = std::make_shared<mesh::MeshSet>(
        rootDir,
        meshPath,
        smoothNormals,
        forceNormals);

    auto future = startLoad(meshSet);
    m_meshes[key] = future;

    return future;
}

std::shared_future<std::shared_ptr<mesh::MeshSet>> ModelRegistry::startLoad(
    std::shared_ptr<mesh::MeshSet> meshSet)
{
    std::promise<std::shared_ptr<mesh::MeshSet>> promise;
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
                    p.set_value(meshSet);
                }
                else {
                    KI_CRITICAL(fmt::format("MODEL_ERROR: Invalid mesh: {}", meshSet->str()));
                    p.set_value(nullptr);
                }
            }
            catch (const std::exception& ex) {
                KI_CRITICAL(fmt::format("MODEL_ERROR: {}", ex.what()));
                lastException = std::current_exception();
                p.set_exception(lastException);
            }
            catch (const std::string& ex) {
                KI_CRITICAL(fmt::format("MODEL_ERROR: {}", ex));
                lastException = std::current_exception();
                p.set_exception(lastException);
            }
            catch (const char* ex) {
                KI_CRITICAL(fmt::format("MODEL_ERROR: {}", ex));
                lastException = std::current_exception();
                p.set_exception(lastException);
            }
            catch (...) {
                KI_CRITICAL(fmt::format("MODEL_ERROR: {}", "UNKNOWN_ERROR"));
                lastException = std::current_exception();
                p.set_exception(lastException);
            }
        }
    };
    th.detach();

    return future;
}
