#pragma once

#include "asset/MaterialVBO.h"
#include "asset/Sprite.h"

#include "backend/DrawOptions.h"

#include "kigl/GLVertexArray.h"

#include "EntityType.h"


struct NodeRenderFlags {
    bool alpha : 1 {false};
    bool blend : 1 {false};
    bool renderBack : 1 {false};
    bool wireframe : 1 {false};

    bool gbuffer : 1 {false};
    bool blendOIT : 1 {false};
    bool depth : 1 {false};

    bool instanced : 1 {false};

    bool mirror : 1 {false};
    bool water : 1 {false};
    bool terrain : 1 {false};
    bool cubeMap : 1 {false};
    bool effect : 1 {false};
    bool skybox : 1 {false};

    bool noShadow : 1 {false};
    bool noSelect : 1 {false};
    bool noReflect : 1 {false};
    bool noRefract : 1 {false};
    // invisible == permanently invisible
    bool invisible : 1 {false};
    bool noFrustum : 1 {false};
    bool noNormals : 1 {false};

    bool tessellation : 1 {false};

    bool staticPhysics : 1 {false};
    bool enforceBounds : 1 {false};
    bool physics : 1 {false};
};

class CustomMaterial;
class Program;
class Registry;
class Mesh;
class Batch;
class RenderContext;


class MeshType final
{
    friend class MeshTypeRegistry;

public:
    MeshType(std::string_view name);
    MeshType(MeshType&& o);
    ~MeshType();

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

    int resolveMaterialIndex() const;

    inline size_t getMaterialCount() const noexcept
    {
        return m_materialVBO.m_materials.size();
    }


    CustomMaterial* getCustomMaterial() { return m_customMaterial.get(); }

    void setCustomMaterial(std::unique_ptr<CustomMaterial> customMaterial) noexcept;

    void prepare(
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

    GLVertexArray* m_vao{ nullptr };

private:
    bool m_prepared : 1 {false};

    Mesh* m_mesh{ nullptr };
    //std::unique_ptr<Mesh> m_deleter;

    std::unique_ptr<CustomMaterial> m_customMaterial{ nullptr };

    GLVertexArray m_privateVAO;
};

// https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
struct MeshTypeComparator {
    bool operator()(const MeshType* a, const MeshType* b) const {
        if (a->m_drawOptions < b->m_drawOptions) return true;
        else if (b->m_drawOptions < a->m_drawOptions) return false;
        return a->m_id < b->m_id;
    }
};
