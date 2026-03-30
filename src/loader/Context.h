#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <atomic>

#include "util/Ref.h"

class AsyncLoader;

namespace loader {
    struct Context {
        Context(
            util::Ref<AsyncLoader> asyncLoader,
            const std::string& sdirName,
            const std::string& fileName);

        std::string str() const noexcept
        {
            return m_fullPath;
        }

    public:
        const std::string m_assetsDir;
        const std::string m_dirName;
        const std::string m_fileName;

        const std::string m_fullPath;
        const std::string m_name;

        std::shared_ptr<std::atomic_bool> m_alive;

        util::Ref<AsyncLoader> m_asyncLoader;
        std::shared_ptr<std::atomic<int>> m_runningCount;
    };
}
