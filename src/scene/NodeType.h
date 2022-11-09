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
    bool sprite = false;
    bool renderBack = false;
    bool noShadow = false;
    bool noSelect = false;
    bool noReflect = false;
    bool noRefract = false;
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

class NodeType final
{
public:
    NodeType();
    ~NodeType();

    const std::string str() const noexcept;

    bool hasReflection() noexcept;
    bool hasRefraction() noexcept;

    Material* findMaterial(std::function<bool(const Material&)> fn) noexcept;
    void modifyMaterials(std::function<void(Material&)> fn) noexcept;

    void prepare(const Assets& assets) noexcept;
    void prepareBatch(Batch& batch) noexcept;

    void bind(const RenderContext& ctx, Shader* shader) noexcept;
    void unbind(const RenderContext& ctx) noexcept;

public:
    const int typeID;

    NodeRenderFlags m_flags;

    std::unique_ptr<Mesh> m_mesh{ nullptr };

    std::string m_initScript;
    std::string m_runScript;

    Shader* m_nodeShader{ nullptr };
    Shader* m_boundShader{ nullptr };

private:
    bool m_prepared = false;
    bool m_preparedBatch = false;
};
