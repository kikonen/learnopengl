#pragma once

#include "asset/Mesh.h"
#include "asset/Shader.h"

#include "kigl/GLVertexArray.h"

#include "scene/Batch.h"


struct NodeRenderFlags {
    bool alpha = false;
    bool blend = false;
    bool mirror = false;
    bool water = false;
    bool terrain = false;
    bool sprite = false;
    bool renderBack = false;
    bool noShadow = false;
    bool noSelect = false;
    bool noReflect = false;
    bool noRefract = false;
    bool noRender = false;
    bool wireframe = false;
    bool instanced = false;
    bool root = false;
    bool origo = false;
    bool noFrustum = false;
    bool cubeMap = false;
};

enum class NodeScriptId {
    init,
    run
};

class RenderContext;
class NodeRegistry;
class Mesh;

class MeshType final
{
    friend class MeshTypeRegistry;

public:
    MeshType(const std::string& name);
    ~MeshType();

    const std::string str() const noexcept;

    void setMesh(std::unique_ptr<Mesh> mesh, bool unique);
    void setMesh(Mesh* mesh);
    const Mesh* getMesh() const;

    void modifyMaterials(std::function<void(Material&)> fn);

    void prepare(
        const Assets& assets,
        NodeRegistry& registry) noexcept;

    void prepareBatch(Batch& batch) noexcept;

    void bind(const RenderContext& ctx, Shader* shader) noexcept;
    void unbind(const RenderContext& ctx) noexcept;

public:
    const int typeID;
    const std::string m_name;

    NodeRenderFlags m_flags;

    std::string m_initScript;
    std::string m_runScript;

    Shader* m_nodeShader{ nullptr };

    MaterialVBO m_materialVBO;

private:
    bool m_prepared = false;
    bool m_preparedBatch = false;

    Mesh* m_mesh{ nullptr };
    std::unique_ptr<Mesh> m_deleter;

    GLVertexArray m_vao;
};
