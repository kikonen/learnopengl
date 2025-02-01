#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <atomic>

class AsyncLoader;

namespace loader {
    struct Context {
        Context(
            std::shared_ptr<std::atomic<bool>> alive,
            std::shared_ptr<AsyncLoader> asyncLoader,
            const std::string& dirName,
            const std::string& fileName);

        std::string str() const noexcept
        {
            return m_fullPath;
        }

    public:
        const std::string m_assetsDir;
        const std::string m_fullPath;
        const std::string m_dirName;
        const std::string m_fileName;

        std::shared_ptr<std::atomic<bool>> m_alive;
        std::shared_ptr<AsyncLoader> m_asyncLoader;
    };
}
