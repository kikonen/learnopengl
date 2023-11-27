#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "asset/Assets.h"

class AsyncLoader;

namespace loader {
    struct Context {
        Context(
            const Assets& assets,
            std::shared_ptr<std::atomic<bool>> alive,
            std::shared_ptr<AsyncLoader> asyncLoader,
            const std::string& filename);

        const std::string str() const noexcept
        {
            return m_filename;
        }

    public:
        const Assets& m_assets;
        const std::string m_filename;
        const std::string m_dirname;

        std::shared_ptr<std::atomic<bool>> m_alive;
        std::shared_ptr<AsyncLoader> m_asyncLoader;

        std::shared_ptr<std::unordered_map<std::string, uuids::uuid>> m_autoIds;
    };
}
