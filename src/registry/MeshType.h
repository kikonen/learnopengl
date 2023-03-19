#pragma once

#include "asset/MaterialVBO.h"

#include "backend/DrawOptions.h"

#include "kigl/GLVertexArray.h"

#include "asset/CustomMaterial.h"

#include "component/ComponentType.h"

#include "registry/EntityType.h"

struct NodeRenderFlags {
    bool alpha = false;
    bool blend = false;
    bool renderBack = false;
    bool wireframe = false;

    bool gbuffer = false;

    bool instanced = false;

    bool mirror = false;
    bool water = false;
    bool cubeMap = false;

    bool noShadow = false;
    bool noSelect = false;
    bool noReflect = false;
    bool noRefract = false;
    // invisible == permanently invisible
    bool invisible = false;
    // noDisplay == temporarily hidden
    bool noDisplay = false;
    bool noFrustum = false;

    bool tessellation = false;

    bool staticPhysics = false;
    bool enforceBounds = false;
    bool physics = false;
};

enum class NodeScriptId {
    init,
    run
};

class Program;
class Registry;
class Mesh;
class Batch;

class MeshType final
{
    friend class MeshTypeRegistry;

public:
    MeshType(const std::string& name);
    ~MeshType();

    const std::string str() const noexcept;

    void setMesh(std::unique_ptr<Mesh> mesh, bool unique);
    void setMesh(Mesh* mesh);

    inline const Mesh* getMesh() const noexcept {
        return m_mesh;
    }

    void modifyMaterials(std::function<void(Material&)> fn);

    int getMaterialIndex() const;
    int getMaterialCount() const;

    CustomMaterial* getCustomMaterial();
    void setCustomMaterial(std::unique_ptr<CustomMaterial> customMaterial)
    {
        m_customMaterial = std::move(customMaterial);
    }

    void prepare(
        const Assets& assets,
        Registry* registry);

    void bind(const RenderContext& ctx);

public:
    const int typeID;
    const std::string m_name;

    ComponentType m_componentType{ ComponentType::origo };
    EntityType m_entityType{ EntityType::origo };

    NodeRenderFlags m_flags;

    int m_priority = { 0 };

    std::string m_script;

    Program* m_program{ nullptr };

    MaterialVBO m_materialVBO;

    backend::DrawOptions m_drawOptions;

    GLVertexArray* m_vao{ nullptr };

private:
    bool m_prepared = false;
    bool m_preparedBatch = false;

    Mesh* m_mesh{ nullptr };
    std::unique_ptr<Mesh> m_deleter;

    std::unique_ptr<CustomMaterial> m_customMaterial{ nullptr };

    GLVertexArray m_privateVAO;
};

// https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
struct MeshTypeComparator {
    bool operator()(const MeshType* a, const MeshType* b) const {
        if (a->m_drawOptions < b->m_drawOptions) return true;
        else if (b->m_drawOptions < a->m_drawOptions) return false;
        return a->typeID < b->typeID;
    }
};
