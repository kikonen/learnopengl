#include "VertexLoader.h"

#include "loader/document.h"
#include "Loaders.h"

#include "loader_util.h"

#include "mesh/PrimitiveMesh.h"
#include "mesh/generator/PrimitiveGenerator.h"


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
                else if (type == "capsule") {
                    data.type = mesh::PrimitiveType::capsule;
                }
                else if (type == "cylinder") {
                    data.type = mesh::PrimitiveType::cylinder;
                }
                else if (type == "capped_cylinder") {
                    data.type = mesh::PrimitiveType::capped_cylinder;
                }
                else if (type == "cone") {
                    data.type = mesh::PrimitiveType::cone;
                }
                else if (type == "capped_cone") {
                    data.type = mesh::PrimitiveType::capped_cone;
                }
                else if (type == "torus") {
                    data.type = mesh::PrimitiveType::torus;
                }
                else if (type == "disk") {
                    data.type = mesh::PrimitiveType::disk;
                }
                else if (type == "spring") {
                    data.type = mesh::PrimitiveType::spring;
                }
                else {
                    reportUnknown("vertex_type", k, v);
                }
            }
            else if (k == "size") {
                data.size = readVec3(v);
            }
            else if (k == "inner_radius") {
                data.inner_radius = readFloat(v);
            }
            else if (k == "radius") {
                data.radius = readFloat(v);
            }
            else if (k == "length") {
                data.length = readFloat(v);
            }
            else if (k == "slices") {
                data.slices = readInt(v);
            }
            else if (k == "segments") {
                data.segments = readInt(v);
            }
            else if (k == "rings") {
                data.rings = readInt(v);
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

            switch (data.type) {
            case mesh::PrimitiveType::points:
                valid &= !(data.vertices.empty() || data.indeces.empty());
                break;
            case mesh::PrimitiveType::lines:
                valid &= !(data.vertices.empty() || data.indeces.empty());
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

        auto generator = mesh::PrimitiveGenerator::get(data.type);

        if (!data.name.empty()) {
            generator.name = data.name;
        }
        if (!data.alias.empty()) {
            generator.alias= data.alias;
        }

        generator.size = data.size;
        generator.inner_radius = data.inner_radius;
        generator.radius = data.radius;
        generator.length = data.length;
        generator.slices = data.slices;
        generator.segments = data.segments;
        generator.rings = data.rings;
        generator.vertices = data.vertices;
        generator.indeces = data.indeces;

        return generator.create();
    }
}
