#include "Context.h"

#include <filesystem>

#include "asset/Assets.h"

#include "engine/AsyncLoader.h"

#include "util/util.h"
#include "util/file.h"

namespace {
    std::string resolveFilePath(
        const std::string& defaultSceneDir,
        const std::string& sceneFile)
    {
        if (util::fileExists(sceneFile))
        {
            return sceneFile;
        }

        return util::joinPath(defaultSceneDir, sceneFile);
    }

    std::string resolveName(
        const std::string& sceneFile)
    {
        std::filesystem::path p{ sceneFile };
        return p.stem().string();
    }
}

namespace loader {
    Context::Context(
        const std::string& defaultSceneDir,
        const std::string& sceneFile)
        : m_alive{ std::make_shared<std::atomic_bool>(true)},
        m_runningCount{ std::make_shared<std::atomic<int>>(0) },
        m_asyncLoader{ util::Ref<AsyncLoader>::create() },
        m_assetsDir{ Assets::get().assetsDir},
        m_defaultSceneDir{ defaultSceneDir },
        m_sceneFile{ sceneFile },
        m_fullPath{ resolveFilePath(defaultSceneDir, sceneFile) },
        m_name{ resolveName(sceneFile) }
    {
    }

    Context::~Context() = default;
}
