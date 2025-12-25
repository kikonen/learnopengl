#pragma once

#include <assimp/LogStream.hpp>

namespace mesh_set
{
    class AssimpLogger : public Assimp::LogStream
    {
    public:
        AssimpLogger();
        ~AssimpLogger();

        void write(const char* message) override;

        static void attach(bool debug);
    };
}
