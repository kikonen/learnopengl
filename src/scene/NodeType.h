#pragma once

#include "asset/Mesh.h"
#include "asset/Shader.h"
#include "scene/RenderContext.h"
#include "scene/Batch.h"


struct NodeRenderFlags {
    bool alpha = false;
    bool blend = false;
    bool light = false;
    bool mirror = false;
    bool water = false;
    bool renderBack = false;
    bool noShadow = false;
    bool batchMode = true;
    bool wireframe = false;
    bool instanced = false;
};

class NodeType final
{
public:
    NodeType();
    ~NodeType();

    std::string str();

    bool hasReflection();
    bool hasRefraction();

    Material* findMaterial(std::function<bool(const Material&)> fn);
    void modifyMaterials(std::function<void(Material&)> fn);

    void prepare(const Assets& assets);
    void bind(const RenderContext& ctx, Shader* shader);
    void unbind(const RenderContext& ctx);

public:
    const int typeID;

    NodeRenderFlags flags;

    glm::vec4 mirrorPlane{ 0 };

    std::unique_ptr<Mesh> mesh{ nullptr };

    std::string initScript;
    std::string runScript;

    Shader* nodeShader{ nullptr };
    Shader* boundShader{ nullptr };

    Batch batch;

private:
    bool m_prepared = false;
};
