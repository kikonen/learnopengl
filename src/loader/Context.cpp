#include "Context.h"

#include "asset/Assets.h"

#include "util/util.h"
#include "util/file.h"

namespace loader {
    Context::Context(
        std::shared_ptr<std::atomic<bool>> alive,
        std::shared_ptr<AsyncLoader> asyncLoader,
        const std::string& dirName,
        const std::string& fileName)
        : m_alive(alive),
        m_asyncLoader(asyncLoader),
        m_assetsDir{ Assets::get().assetsDir},
        m_dirName{ dirName },
        m_fileName{ fileName },
        m_fullPath{ util::joinPath(dirName, fileName) }
    {
    }
}
