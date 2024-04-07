#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "asset/Material.h"

#include "mesh/Index.h"
#include "mesh/Vertex.h"
#include "mesh/Mesh.h"
#include "mesh/ModelVBO.h"

namespace mesh {
    class ModelMesh final : public Mesh {
        friend class ModelLoader;
        friend class AssimpLoader;
        friend class ObjectLoader;

        friend class ModelVBO;
        friend class ModelMaterialInit;

    public:
        ModelMesh(
            std::string_view meshName,
            std::string_view rootDir);

        ModelMesh(
            std::string_view meshName,
            std::string_view rootDir,
            std::string_view meshPath);

        virtual ~ModelMesh();

        virtual std::string str() const noexcept override;
        virtual bool isValid() const noexcept override
        {
            return !m_vertices.empty() && !m_indeces.empty();
        }

        virtual const AABB calculateAABB() const override;

        virtual const std::vector<Material>& getMaterials() const override;

        virtual kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) override;

        virtual void prepareMaterials(
            MaterialSet& materialSet) override;

        virtual void prepareLod(
            mesh::LodMesh& lodMesh);

        virtual void prepareDrawOptions(
            backend::DrawOptions& drawOptions) override;

    public:
        const std::string m_meshName;
        const std::string m_rootDir;
        const std::string m_meshPath;

        std::string m_filePath;
        std::string m_fileExt;

    protected:
        uint32_t m_indexCount{ 0 };
        std::vector<Index> m_indeces;
        std::vector<Vertex> m_vertices;
        std::vector<Material> m_materials;

    private:
        bool m_loaded{ false };
        bool m_valid{ false };

        ModelVBO m_vertexVBO;
    };
}
