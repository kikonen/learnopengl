#pragma once

#include <memory>

#include "BaseLoader.h"

#include "MeshData.h"
#include "VertexData.h"

namespace mesh
{
    class Mesh;
}

namespace loader {
    class VertexLoader : public BaseLoader
    {
    public:
        VertexLoader(
            Context ctx);

        void load(
            const loader::DocNode& node,
            VertexData& data) const;

        void loadVertices(
            const loader::DocNode& node,
            std::vector<glm::vec3>& vertices) const;

        void loadIndeces(
            const loader::DocNode& node,
            std::vector<int>& indeces) const;

        std::shared_ptr<mesh::Mesh> createMesh(
            const MeshData& meshData,
            const VertexData& data,
            Loaders& loaders) const;

    private:
        std::shared_ptr<mesh::Mesh> createMesh(
            std::string defaultName,
            const MeshData& meshData,
            const VertexData& data,
            Loaders& loaders) const;
    };
}
