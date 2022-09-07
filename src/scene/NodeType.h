#pragma once

#include "asset/Mesh.h"
#include "asset/Shader.h"
#include "scene/RenderContext.h"
#include "scene/Batch.h"

class NodeType final
{
public:
    static int nextID();
    static void setBaseID(int baseId);

    NodeType(int typeID, std::shared_ptr<Shader> defaultShader = nullptr);
    ~NodeType();

    bool hasReflection();
    bool hasRefraction();

    std::shared_ptr<Material> findMaterial(std::function<bool(Material&)> fn);
    void modifyMaterials(std::function<void(Material&)> fn);

    void prepare(const Assets& assets);
    Shader* bind(const RenderContext& ctx, Shader* shader);
    void unbind(const RenderContext& ctx);

public:
    const int typeID;

    bool blend = false;
    bool light = false;
    bool mirror = false;
    bool water = false;
    bool renderBack = false;
    bool noShadow = false;
    bool batchMode = true;
    bool wireframe = false;

    glm::vec4 mirrorPlane{ 0 };

    std::unique_ptr<Mesh> mesh = nullptr;
    std::shared_ptr<Shader> defaultShader = nullptr;
    Shader* boundShader = nullptr;

    Batch batch;
private:
};


