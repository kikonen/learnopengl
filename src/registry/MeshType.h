#pragma once

#include "asset/MaterialVBO.h"

#include "backend/DrawOptions.h"

#include "kigl/GLVertexArray.h"

#include "EntityType.h"


struct NodeRenderFlags {
    bool alpha = false;
    bool blend = false;
    bool mirror = false;
    bool water = false;
    bool renderBack = false;
    bool noShadow = false;
    bool noSelect = false;
    bool noReflect = false;
    bool noRefract = false;
    bool noRender = false;
    bool noDisplay = false;
    bool wireframe = false;
    bool instanced = false;
    bool root = false;
    bool noFrustum = false;
    bool cubeMap = false;
};

enum class NodeScriptId {
    init,
    run
};

class Shader;
class RenderContext;
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

    const Mesh* getMesh() const {
        return m_mesh;
    }

    void modifyMaterials(std::function<void(Material&)> fn);

    int getMaterialIndex() const;
    int getMaterialCount() const;

    void prepare(
        const Assets& assets,
        Registry* registry);

public:
    const int typeID;
    const std::string m_name;

    EntityType m_entityType;
    NodeRenderFlags m_flags;

    std::string m_initScript;
    std::string m_runScript;

    Shader* m_nodeShader{ nullptr };

    MaterialVBO m_materialVBO;

    backend::DrawOptions m_drawOptions;

    GLVertexArray* m_vao{ nullptr };

private:
    bool m_prepared = false;
    bool m_preparedBatch = false;

    Mesh* m_mesh{ nullptr };
    std::unique_ptr<Mesh> m_deleter;

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
