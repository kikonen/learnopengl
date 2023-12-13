#include "Context.h"

#include "util/Util.h"

namespace loader {
    Context::Context(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive,
        std::shared_ptr<AsyncLoader> asyncLoader,
        const std::string& filename)
        :m_assets(assets),
        m_alive(alive),
        m_asyncLoader(asyncLoader),
        m_filename(filename),
        m_dirname(util::dirName(filename))
    {
        m_autoIds = std::make_shared<std::unordered_map<std::string, uuids::uuid>>();
    }
}
