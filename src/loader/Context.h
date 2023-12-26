#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <atomic>

#include "asset/Assets.h"

class AsyncLoader;

namespace loader {
    struct Context {
        Context(
            const Assets& assets,
            std::shared_ptr<std::atomic<bool>> alive,
            std::shared_ptr<AsyncLoader> asyncLoader,
            const std::string& dirName,
            const std::string& fileName);

        const std::string str() const noexcept
        {
            return m_fullPath;
        }

    public:
        const Assets& m_assets;

        const std::string m_fullPath;
        const std::string m_dirName;
        const std::string m_fileName;

        std::shared_ptr<std::atomic<bool>> m_alive;
        std::shared_ptr<AsyncLoader> m_asyncLoader;

        std::shared_ptr<std::unordered_map<std::string, uuids::uuid>> m_autoIds;
    };
}
