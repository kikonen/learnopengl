#include "Context.h"

#include "util/Util.h"

namespace loader {
    Context::Context(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive,
        std::shared_ptr<AsyncLoader> asyncLoader,
        const std::string& dirName,
        const std::string& fileName)
        :m_assets(assets),
        m_alive(alive),
        m_asyncLoader(asyncLoader),
        m_dirName(dirName),
        m_fileName(fileName),
        m_fullPath(util::joinPath(dirName, fileName))
    {
    }
}
