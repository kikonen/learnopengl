#pragma once

#include "glm/glm.hpp"

#include "asset/Assets.h"

class AsyncLoader;

class ShaderRegistry;
class NodeRegistry;
class MeshTypeRegistry;
class ModelRegistry;
class MaterialRegistry;

class TestSceneSetup final
{
public:
    TestSceneSetup(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive,
        std::shared_ptr<AsyncLoader> asyncLoader);

    void setup(
        std::shared_ptr<ShaderRegistry> shaderRegistry,
        std::shared_ptr<NodeRegistry> nodeRegistry,
        std::shared_ptr<MeshTypeRegistry> typeRegistry,
        std::shared_ptr<MaterialRegistry> materialRegistry,
        std::shared_ptr<ModelRegistry> modelRegistry);

private:
    void setupEffectExplosion();

    void setupViewport1();
private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::shared_ptr<AsyncLoader> m_asyncLoader;

    std::shared_ptr<ShaderRegistry> m_shaderRegistry{ nullptr };
    std::shared_ptr<NodeRegistry> m_nodeRegistry{ nullptr };
    std::shared_ptr<MeshTypeRegistry> m_typeRegistry{ nullptr };
    std::shared_ptr<MaterialRegistry> m_materialRegistry{ nullptr };
    std::shared_ptr<ModelRegistry> m_modelRegistry{ nullptr };
};
