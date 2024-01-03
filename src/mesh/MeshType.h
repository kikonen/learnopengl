#pragma once

#include "asset/Sprite.h"

#include "mesh/MaterialVBO.h"

#include "backend/DrawOptions.h"

#include "kigl/GLVertexArray.h"

#include "EntityType.h"

#include "NodeRenderFlags.h"

class CustomMaterial;
class Program;
class Registry;
class Batch;
class RenderContext;
class MeshTypeRegistry;

namespace mesh {
    class Mesh;

    class MeshType final
    {
        friend class MeshTypeRegistry;

    public:
        MeshType(std::string_view name);
        MeshType(MeshType&& o) noexcept;
        ~MeshType();

        inline bool isReady() const { return m_preparedView; }

        const std::string str() const noexcept;

        //void setMesh(std::unique_ptr<Mesh> mesh, bool unique);
        void setMesh(Mesh* mesh);

        inline const Mesh* getMesh() const noexcept {
            return m_mesh;
        }

        void modifyMaterials(std::function<void(Material&)> fn);

        inline int getMaterialIndex() const noexcept
        {
            return m_materialIndex;
        }

        CustomMaterial* getCustomMaterial() { return m_customMaterial.get(); }

        void setCustomMaterial(std::unique_ptr<CustomMaterial> customMaterial) noexcept;

        void prepare(
            const Assets& assets,
            Registry* registry);

        void prepareRT(
            const Assets& assets,
            Registry* registry);

        void bind(const RenderContext& ctx);

    public:
        ki::type_id m_id{ 0 };
        const std::string m_name;

        EntityType m_entityType{ EntityType::origo };
        NodeRenderFlags m_flags;

        ki::size_t8 m_priority{ 0 };

        Program* m_program{ nullptr };
        Program* m_depthProgram{ nullptr };

        MaterialVBO m_materialVBO;
        Sprite m_sprite;

        int m_materialIndex{ 0 };

        backend::DrawOptions m_drawOptions;

        kigl::GLVertexArray* m_vao{ nullptr };

    private:
        bool m_prepared : 1 {false};
        bool m_preparedView : 1 {false};

        Mesh* m_mesh{ nullptr };
        //std::unique_ptr<Mesh> m_deleter;

        std::unique_ptr<CustomMaterial> m_customMaterial{ nullptr };

        kigl::GLVertexArray m_privateVAO;
    };

    // https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
    struct MeshTypeComparator {
        bool operator()(const MeshType* a, const MeshType* b) const {
            if (a->m_drawOptions < b->m_drawOptions) return true;
            else if (b->m_drawOptions < a->m_drawOptions) return false;
            return a->m_id < b->m_id;
        }
    };
}
