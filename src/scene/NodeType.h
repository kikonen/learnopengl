#pragma once

#include "asset/Mesh.h"
#include "asset/Shader.h"
#include "scene/RenderContext.h"
#include "scene/Batch.h"


struct NodeRenderFlags {
    bool alpha = false;
    bool blend = false;
    bool mirror = false;
    bool water = false;
    bool terrain = false;
    bool renderBack = false;
    bool noShadow = false;
    bool wireframe = false;
    bool instanced = false;
    bool root = false;
    bool origo = false;
    bool cubeMap = false;
};

enum class NodeScriptId {
    init,
    run
};

class NodeType final
{
public:
    NodeType();
    ~NodeType();

    const std::string str() const;

    bool hasReflection();
    bool hasRefraction();

    Material* findMaterial(std::function<bool(const Material&)> fn);
    void modifyMaterials(std::function<void(Material&)> fn);

    void prepare(const Assets& assets);
    void bind(const RenderContext& ctx, Shader* shader);
    void unbind(const RenderContext& ctx);

public:
    const int typeID;

    NodeRenderFlags m_flags;

    glm::vec4 m_mirrorPlane{ 0 };

    std::unique_ptr<Mesh> m_mesh{ nullptr };

    std::string m_initScript;
    std::string m_runScript;

    Shader* m_nodeShader{ nullptr };
    Shader* m_boundShader{ nullptr };

    Batch m_batch;

private:
    bool m_prepared = false;
};
