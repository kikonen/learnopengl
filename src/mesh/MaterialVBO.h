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

        virtual ~MaterialVBO() = default;

        void setMaterials(const std::vector<Material>& materials);
        const std::vector<Material>& getMaterials() const noexcept;

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

    public:
        size_t m_bufferIndex{ 0 };

        std::vector<Material> m_materials;
        std::vector<GLuint> m_indeces;

    private:
        bool m_prepared = false;

        std::unique_ptr<Material> m_defaultMaterial;
        bool m_useDefaultMaterial{ false };
        bool m_forceDefaultMaterial{ false };
    };
}
