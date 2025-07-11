#include "VertexLoader.h"

#include "loader/document.h"
#include "Loaders.h"

#include "loader_util.h"

#include "mesh/PrimitiveMesh.h"
#include "mesh/generator/PrimitiveGenerator.h"


namespace loader {
    VertexLoader::VertexLoader(
        std::shared_ptr<Context> ctx)
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
                else if (type == "ray") {
                    data.type = mesh::PrimitiveType::ray;
                }
                else if (type == "bezier") {
                    data.type = mesh::PrimitiveType::bezier;
                }
                else if (type == "plane") {
                    data.type = mesh::PrimitiveType::plane;
                }
                else if (type == "quad") {
                    data.type = mesh::PrimitiveType::quad;
                }
                else if (type == "plane_grid") {
                    data.type = mesh::PrimitiveType::plane_grid;
                }
                else if (type == "box" || type == "cube") {
                    data.type = mesh::PrimitiveType::box;
                }
                else if (type == "rounded_box") {
                    data.type = mesh::PrimitiveType::rounded_box;
                }
                else if (type == "dodeca_hedron") {
                    data.type = mesh::PrimitiveType::dodeca_hedron;
                }
                else if (type == "icosa_hedron") {
                    data.type = mesh::PrimitiveType::icosa_hedron;
                }
                else if (type == "ico_sphere") {
                    data.type = mesh::PrimitiveType::ico_sphere;
                }
                else if (type == "sphere") {
                    data.type = mesh::PrimitiveType::sphere;
                }
                else if (type == "ball") {
                    data.type = mesh::PrimitiveType::sphere;
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
                else if (type == "spherical_cone") {
                    data.type = mesh::PrimitiveType::spherical_cone;
                }
                else if (type == "tube") {
                    data.type = mesh::PrimitiveType::tube;
                }
                else if (type == "capped_tube") {
                    data.type = mesh::PrimitiveType::capped_tube;
                }
                else if (type == "torus") {
                    data.type = mesh::PrimitiveType::torus;
                }
                else if (type == "torus_knot") {
                    data.type = mesh::PrimitiveType::torus_knot;
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
                data.has_size = true;
            }
            else if (k == "inner_radius") {
                data.inner_radius = readFloat(v);
                data.has_inner_radius = true;
            }
            else if (k == "radius") {
                data.radius = readFloat(v);
                data.has_radius = true;
            }
            else if (k == "length") {
                data.length = readFloat(v);
                data.has_length = true;
            }
            else if (k == "p") {
                data.p = readInt(v);
                data.has_p= true;
            }
            else if (k == "q") {
                data.q = readInt(v);
                data.has_q = true;
            }
            else if (k == "slices") {
                data.slices = readInt(v);
                data.has_slices = true;
            }
            else if (k == "segments") {
                data.segments = readUVec3(v);
                data.has_segments = true;
            }
            else if (k == "sub_segments") {
                data.sub_segments = readUVec3(v);
                data.has_sub_segments = true;
            }
            else if (k == "rings") {
                data.rings = readInt(v);
                data.has_rings = true;
            }
            else if (k == "origin") {
                data.origin = readVec3(v);
                data.has_origin = true;
            }
            else if (k == "dir") {
                data.dir = readVec3(v);
                data.has_dir = true;
            }
            else if (k == "bezier_d0") {
                loadVertices(v, data.bezier_d0);
            }
            else if (k == "bezier_d1") {
                loadVertices(v, data.bezier_d1);
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
            case mesh::PrimitiveType::bezier:
                valid &= data.bezier_d0.size() > 1;
                valid &= data.bezier_d1.size() > 1;
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

    std::shared_ptr<mesh::Mesh> VertexLoader::createMesh(
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

        if (data.has_size) generator.size = data.size;
        if (data.has_inner_radius) generator.inner_radius = data.inner_radius;
        if (data.has_radius) generator.radius = data.radius;
        if (data.has_length) generator.length = data.length;
        if (data.has_p) generator.p = data.p;
        if (data.has_q) generator.q = data.q;
        if (data.has_slices) generator.slices = data.slices;
        if (data.has_segments) generator.segments = data.segments;
        if (data.has_sub_segments) generator.sub_segments = data.sub_segments;
        if (data.has_rings) generator.rings = data.rings;
        if (data.has_origin) generator.origin = data.origin;
        if (data.has_dir) generator.dir = data.dir;

        generator.bezier_d0 = data.bezier_d0;
        generator.bezier_d1 = data.bezier_d1;

        generator.vertices = data.vertices;
        generator.indeces = data.indeces;

        auto mesh = generator.create();
        if (!mesh->getMaterial()) {
            const auto& material = Material::createMaterial(BasicMaterial::yellow);
            mesh->setMaterial(&material);
        }
        return mesh;
    }
}
