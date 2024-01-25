#pragma once

#include <functional>

#include "backend/DrawOptions.h"

#include "asset/Material.h"

#include "kigl/GLVertexArray.h"

#include "EntityType.h"

#include "NodeRenderFlags.h"

namespace pool {
    class TypeHandle;
}

namespace render {
    struct MeshTypeKey;
    struct MeshTypeComparator;
    class Batch;
}

class Sprite;

struct PrepareContext;

class CustomMaterial;
class Program;
class Registry;
class RenderContext;
class MeshTypeRegistry;

namespace mesh {
    class Mesh;
    class MaterialVBO;

    class MeshType final
    {
        friend class pool::TypeHandle;
        friend class MeshTypeRegistry;
        friend struct render::MeshTypeComparator;
        friend struct render::MeshTypeKey;

    public:
        MeshType();
        MeshType(MeshType& o) = delete;
        MeshType(const MeshType& o) = delete;
        MeshType(MeshType&& o) noexcept;
        ~MeshType();

        MeshType& operator=(const MeshType& o) = delete;
        MeshType& operator=(MeshType&& o) = delete;

        inline ki::type_id getId() const noexcept { return m_id; }
        pool::TypeHandle toHandle() const noexcept;

        const std::string& getName() const noexcept { return m_name; }
        void setName(std::string_view name) noexcept {
            m_name = name;
        }

        inline bool isReady() const noexcept { return m_preparedView; }

        const std::string str() const noexcept;

        void setMesh(std::unique_ptr<Mesh> mesh, bool unique);
        void setMesh(Mesh* mesh);

        inline const Mesh* getMesh() const noexcept {
            return m_mesh;
        }

        void modifyMaterials(std::function<void(Material&)> fn);

        inline int getMaterialIndex() const noexcept
        {
            return m_materialIndex;
        }

        template<typename T>
        inline T* getCustomMaterial() const noexcept {
            return dynamic_cast<T*>(m_customMaterial.get());
        }

        void setCustomMaterial(std::unique_ptr<CustomMaterial> customMaterial) noexcept;

        void prepare(
            const PrepareContext& ctx);

        void prepareRT(
            const PrepareContext& ctx);

        void bind(const RenderContext& ctx);

        inline const kigl::GLVertexArray* getVAO() const noexcept {
            return m_vao;
        }

        inline const backend::DrawOptions& getDrawOptions() const noexcept {
            return m_drawOptions;
        }

        inline backend::DrawOptions& modifyDrawOptions() noexcept {
            return m_drawOptions;
        }

    public:
        EntityType m_entityType{ EntityType::origo };
        NodeRenderFlags m_flags;

        // NOTE KI *BIGGER* values rendered first (can be negative)
        uint8_t m_priority{ 0 };

        Program* m_program{ nullptr };
        Program* m_shadowProgram{ nullptr };
        Program* m_preDepthProgram{ nullptr };

        std::unique_ptr<MaterialVBO> m_materialVBO{ nullptr };
        std::unique_ptr<Sprite> m_sprite{ nullptr };

        int m_materialIndex{ 0 };

    private:
        ki::type_id m_id{ 0 };
        uint32_t m_handleIndex;

        std::string m_name;

        bool m_prepared : 1 {false};
        bool m_preparedView : 1 {false};

        kigl::GLVertexArray* m_vao{ nullptr };
        backend::DrawOptions m_drawOptions;

        Mesh* m_mesh{ nullptr };
        std::unique_ptr<Mesh> m_deleter;

        std::unique_ptr<CustomMaterial> m_customMaterial{ nullptr };

        kigl::GLVertexArray m_privateVAO;
    };
}
