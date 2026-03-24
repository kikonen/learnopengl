#pragma once

#include <memory>

#include "util/Ref.h"

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
            const std::shared_ptr<Context>& ctx);

        void load(
            const loader::DocNode& node,
            VertexData& data) const;

        void loadVertices(
            const loader::DocNode& node,
            std::vector<glm::vec3>& vertices) const;

        void loadIndeces(
            const loader::DocNode& node,
            std::vector<int>& indeces) const;

        util::Ref<mesh::Mesh> createMesh(
            const MeshData& meshData,
            const VertexData& data,
            Loaders& loaders) const;

    private:
        util::Ref<mesh::Mesh> createMesh(
            std::string defaultName,
            const MeshData& meshData,
            const VertexData& data,
            Loaders& loaders) const;
    };
}
