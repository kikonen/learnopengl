#pragma once

#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "asset/Material.h"

namespace mesh {
    class Mesh;

    class MaterialVBO {
        friend class MeshType;

    public:
        MaterialVBO() = default;
        MaterialVBO(MaterialVBO&& o);

        virtual ~MaterialVBO();

        void setMaterials(const std::vector<Material>& materials);

        const std::vector<Material>& getMaterials() const noexcept
        {
            return m_materials;
        }

        std::vector<Material>& modifyMaterials() noexcept
        {
            return m_materials;
        }

        const std::vector<GLuint>& getIndeces() const noexcept
        {
            return m_indeces;
        }

        std::vector<GLuint>& modifyIndeces() noexcept
        {
            return m_indeces;
        }

        inline size_t getMaterialCount() const noexcept
        {
            return m_materials.size();
        }

        const Material& getFirst() const noexcept;

        bool isSingle() const noexcept { return m_indeces.empty(); }

        void setDefaultMaterial(
            const Material& material,
            bool useDefaultMaterial,
            bool forceDefaultMaterial
        );

        Material* getDefaultMaterial() const;
        bool isUseDefaultMaterial() const { return m_useDefaultMaterial; };
        bool isForceDefaultMaterial() const { return m_forceDefaultMaterial; };

        int resolveMaterialIndex() const;

    public:
        size_t m_bufferIndex{ 0 };

    private:
        bool m_prepared = false;

        std::vector<Material> m_materials;
        std::vector<GLuint> m_indeces;

        std::unique_ptr<Material> m_defaultMaterial;
        bool m_useDefaultMaterial{ false };
        bool m_forceDefaultMaterial{ false };
    };
}
