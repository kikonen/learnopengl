#include "Context.h"

#include "asset/Assets.h"

#include "util/util.h"
#include "util/file.h"

namespace loader {
    Context::Context(
        std::shared_ptr<AsyncLoader> asyncLoader,
        const std::string& dirName,
        const std::string& fileName)
        : m_alive{ std::make_shared<std::atomic<bool>>(true)},
        m_runningCount{ std::make_shared<std::atomic<int>>(0) },
        m_asyncLoader(asyncLoader),
        m_assetsDir{ Assets::get().assetsDir},
        m_dirName{ dirName },
        m_fileName{ fileName },
        m_fullPath{ util::joinPath(dirName, fileName) }
    {
    }
}
