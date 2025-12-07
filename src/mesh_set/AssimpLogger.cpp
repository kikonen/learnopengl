#include "AssimpLogger.h"

#include "util/Log.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/Logger.hpp>

namespace mesh_set
{
    AssimpLogger::AssimpLogger() = default;
    AssimpLogger::~AssimpLogger() = default;

    void AssimpLogger::write(const char* message)
    {
        KI_INFO_OUT(message);
    }

    void AssimpLogger::attach()
    {
        Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
        Assimp::DefaultLogger::get()->attachStream(
            new AssimpLogger(),
            Assimp::Logger::Info | Assimp::Logger::Warn | Assimp::Logger::Err
        );
    }
}
