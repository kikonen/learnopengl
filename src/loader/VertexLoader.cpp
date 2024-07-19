#include "VertexLoader.h"

#include "loader/document.h"
#include "Loaders.h"

#include "loader_util.h"

#include "mesh/PrimitiveMesh.h"


namespace loader {
    VertexLoader::VertexLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void VertexLoader::load(
        const loader::DocNode& node,
        VertexData& data) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "type" || k == "xtype") {
                std::string type = readString(v);
                if (type == "points") {
                    data.type = mesh::PrimitiveType::points;
                }
                else if (type == "lines") {
                    data.type = mesh::PrimitiveType::lines;
                }
                else if (type == "line_strip") {
                    data.type = mesh::PrimitiveType::line_strip;
                }
                else {
                    reportUnknown("vertex_type", k, v);
                }
            }
            else if (k == "vertices") {
                loadVertices(v, data.vertices);
            }
            else if (k == "indeces") {
                loadIndeces(v, data.indeces);
            }
            else {
                reportUnknown("vertex_entry", k, v);
            }
        }

        {
            bool valid = true;
            valid = data.type != mesh::PrimitiveType::none;
            valid &= !(data.vertices.empty() || data.indeces.empty());

            switch (data.type) {
            case mesh::PrimitiveType::points:
                break;
            case mesh::PrimitiveType::lines:
                valid &= data.indeces.size() > 1;
                valid &= data.indeces.size() % 2 == 0;
                break;
            }

            data.valid = valid;
        }
    }

    void VertexLoader::loadVertices(
        const loader::DocNode& node,
        std::vector<glm::vec3>& vertices) const
    {
        for (const auto& entry : node.getNodes()) {
            const auto& vertex = readVec3(entry);
            vertices.push_back(vertex);
        }
    }

    void VertexLoader::loadIndeces(
        const loader::DocNode& node,
        std::vector<int>& indeces) const
    {
        if (node.isSequence()) {
            for (const auto& entry : node.getNodes()) {
                const auto& index = readInt(entry);
                indeces.push_back(index);
            }
        }
        else {
            indeces.push_back(readInt(node));
        }

    }

    std::unique_ptr<mesh::Mesh> VertexLoader::createMesh(
        const MeshData& meshData,
        const VertexData& data,
        Loaders& loaders) const
    {
        if (!data.valid) return nullptr;

        switch (data.type) {
        case mesh::PrimitiveType::points:
            return createPrimitiveMesh(meshData, data, loaders);
        case mesh::PrimitiveType::lines:
            return createPrimitiveMesh(meshData, data, loaders);
        }

        return nullptr;
    }

    std::unique_ptr<mesh::Mesh> VertexLoader::createPrimitiveMesh(
        const MeshData& meshData,
        const VertexData& data,
        Loaders& loaders) const
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>();

        mesh->m_type = data.type;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        vertices.reserve(data.vertices.size());
        indeces.reserve(data.indeces.size());

        for (auto& v : data.vertices) {
            auto& vertex = vertices.emplace_back();
            vertex.pos = v;
        }

        for (auto& v : data.indeces) {
            indeces.push_back(v);
        }

        return mesh;
    }
}
