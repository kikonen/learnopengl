#pragma once

#include "backend/DrawOptions.h"

#include "asset/Material.h"

#include "kigl/GLVertexArray.h"

#include "text/size.h"

#include "EntityType.h"

#include "NodeRenderFlags.h"

namespace render {
    struct MeshTypeKey;
    struct MeshTypeComparator;
    class Batch;
}

class Sprite;

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
        friend class MeshTypeRegistry;
        friend struct render::MeshTypeComparator;
        friend struct render::MeshTypeKey;

    public:
        MeshType(std::string_view name);
        MeshType(MeshType& o) = delete;
        MeshType& operator=(MeshType& o) = delete;
        MeshType& operator=(MeshType&& o) = delete;
        MeshType(MeshType&& o) noexcept;
        ~MeshType();

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
            const Assets& assets,
            Registry* registry);

        void prepareRT(
            const Assets& assets,
            Registry* registry);

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
        ki::type_id m_id{ 0 };
        const std::string m_name;

        EntityType m_entityType{ EntityType::origo };
        NodeRenderFlags m_flags;

        ki::size_t8 m_priority{ 0 };

        Program* m_program{ nullptr };
        Program* m_depthProgram{ nullptr };

        std::unique_ptr<MaterialVBO> m_materialVBO{ nullptr };
        std::unique_ptr<Sprite> m_sprite{ nullptr };

        int m_materialIndex{ 0 };

        text::font_id m_fontId{ 0 };

    private:
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
