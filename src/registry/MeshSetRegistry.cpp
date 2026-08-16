#include "MeshSetRegistry.h"

#include <algorithm>

#include <fmt/format.h>

#include "util/util.h"
#include "util/file.h"
#include "util/Log.h"
#include "util/Ref.h"

#include "asset/Assets.h"

#include "mesh/MeshSet.h"
#include "mesh/ModelMesh.h"

#include "mesh/vao/TexturedVAO.h"
#include "mesh/vao/SkinnedVAO.h"

#include "mesh_set/AssimpImporter.h"
#include "mesh_set/AnimationImporter.h"

#include "render/RenderContext.h"

namespace
{
    thread_local std::exception_ptr lastException = nullptr;

    static MeshSetRegistry* s_registry{ nullptr };
}

void MeshSetRegistry::init() noexcept
{
    assert(!s_registry);
    s_registry = new MeshSetRegistry();
}

void MeshSetRegistry::release() noexcept
{
    auto* s = s_registry;
    s_registry = nullptr;
    delete s;
}

MeshSetRegistry& MeshSetRegistry::get() noexcept
{
    assert(s_registry);
    return *s_registry;
}

MeshSetRegistry::MeshSetRegistry()
{
    clear();
}

MeshSetRegistry::~MeshSetRegistry() {
    clear();
}

void MeshSetRegistry::clear()
{
    m_meshes.clear();
}

void MeshSetRegistry::prepare(
    const std::shared_ptr<std::atomic_bool>& alive)
{
    m_alive = alive;
}

std::shared_future<util::Ref<mesh::MeshSet>> MeshSetRegistry::getMeshSet(
    std::string_view id,
    std::string_view rootDir,
    std::string_view meshPath,
    bool smoothNormals,
    bool forceNormals,
    const std::vector<animation::AnimationPath>& animationPaths)
{
    if (!*m_alive) return {};

    std::lock_guard lock(m_meshes_lock);

    // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
    std::string baseKey = fmt::format(
        "{}_{}_{}_{}_{}",
        id,
        rootDir,
        meshPath,
        smoothNormals,
        forceNormals);

    std::string animKey;
    {
        std::vector<std::string> paths;
        for (const auto& animationPath : animationPaths) {
            paths.push_back(fmt::format("{}:{}", animationPath.animationPrefix, animationPath.path));
        }
        std::sort(paths.begin(), paths.end());
        std::string animKey = util::join(paths, "_");
    }
    std::string key = fmt::format("{}_{}", baseKey, animKey);

    {
        auto e = m_meshes.find(key);
        if (e != m_meshes.end())
            return e->second;
    }

    auto meshSet = util::Ref<mesh::MeshSet>::create(
        id,
        rootDir,
        meshPath,
        smoothNormals,
        forceNormals,
        animationPaths);

    auto future = startLoad(meshSet);
    m_meshes[key] = future;

    return future;
}

std::shared_future<util::Ref<mesh::MeshSet>> MeshSetRegistry::startLoad(
    util::Ref<mesh::MeshSet> meshSet)
{
    std::promise<util::Ref<mesh::MeshSet>> promise;
    auto future = promise.get_future().share();

    // NOTE KI use thread instead of std::async since std::future blocking/cleanup is problematic
    // https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
    auto th = std::thread{
        [this, meshSet, p = std::move(promise)]() mutable {
            try {
                const auto assets = Assets::get();

                KI_DEBUG(fmt::format("MESH_SET::START_IMPORTER: {}", meshSet->str()));

                std::unique_ptr<mesh_set::MeshSetImporter> importer;

                if (assets.assimpImporterEnabled) {
                    importer = std::make_unique<mesh_set::AssimpImporter>(m_alive, assets.assimpDebug);
                }
                else {
                    throw "MESH_SET::NO_IMPORTER";
                }

                auto loaded = importer->load(*meshSet, m_defaultMaterial.get(), m_forceDefaultMaterial);

                // NOTE KI if not valid then null; avoids internal errors in render logic
                if (loaded) {
                    loadAnimations(meshSet);
                    p.set_value(meshSet);
                }
                else {
                    KI_CRITICAL(fmt::format("MESH_SET: Invalid mesh: {}", meshSet->str()));
                    p.set_value(nullptr);
                }
            }
            catch (const std::exception& ex) {
                KI_CRITICAL(fmt::format("MESH_SET: {}", ex.what()));
                lastException = std::current_exception();
                p.set_exception(lastException);
            }
            catch (const std::string& ex) {
                KI_CRITICAL(fmt::format("MESH_SET: {}", ex));
                lastException = std::current_exception();
                p.set_exception(lastException);
            }
            catch (const char* ex) {
                KI_CRITICAL(fmt::format("MESH_SET: {}", ex));
                lastException = std::current_exception();
                p.set_exception(lastException);
            }
            catch (...) {
                KI_CRITICAL(fmt::format("MESH_SET: {}", "UNKNOWN_ERROR"));
                lastException = std::current_exception();
                p.set_exception(lastException);
            }
        }
    };
    th.detach();

    return future;
}

void MeshSetRegistry::loadAnimations(
    util::Ref<mesh::MeshSet>& meshSet)
{
    for (const auto& animationPath : meshSet->m_animationPaths) {
        loadAnimation(meshSet, animationPath);
    }
}

void MeshSetRegistry::loadAnimation(
    util::Ref<mesh::MeshSet>& meshSet,
    const animation::AnimationPath& animationPath)
{
    if (animationPath.empty()) return;

    // resolve path
    std::string filePath;
    {
        {
            filePath = util::joinPathExt(
                meshSet->m_rootDir,
                meshSet->m_dir,
                animationPath.path, "");
        }

        if (!util::fileExists(filePath)) {
            filePath = util::joinPath(
                meshSet->m_rootDir,
                animationPath.path);
        }
    }

    for (auto& mesh : meshSet->getMeshes()) {
        const auto& rig = mesh->getRig();
        if (!rig) continue;

        mesh_set::AnimationImporter importer{};
        importer.loadAnimations(
            *rig,
            animationPath.animationPrefix,
            filePath);
    }
}
