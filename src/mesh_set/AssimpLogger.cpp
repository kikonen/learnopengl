#include "AssimpLogger.h"

#include <mutex>
#include <regex>

#include "util/Log.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/Logger.hpp>

namespace
{
    bool g_attached{ false };
    std::mutex g_mutex;
}

namespace mesh_set
{
    AssimpLogger::AssimpLogger() = default;
    AssimpLogger::~AssimpLogger() = default;

    void AssimpLogger::write(const char* message)
    {
        std::string msg{ message };
        msg.resize(msg.size() - 1);
        KI_INFO_OUT(msg);
    }

    void AssimpLogger::attach(bool debug)
    {
        std::lock_guard lock{ g_mutex };

        if (g_attached) return;
        g_attached = true;

        Assimp::Logger::LogSeverity severiry = Assimp::Logger::NORMAL;
        unsigned int level = 0 |
            Assimp::Logger::Warn |
            Assimp::Logger::Err;

        if (debug) {
            severiry = Assimp::Logger::VERBOSE;
            level = 0 |
                Assimp::Logger::Debugging |
                Assimp::Logger::Info |
                Assimp::Logger::Warn |
                Assimp::Logger::Err;
        }

        Assimp::DefaultLogger::create("", severiry);
        Assimp::DefaultLogger::get()->attachStream(
            new AssimpLogger(),
            level
        );
    }
}
