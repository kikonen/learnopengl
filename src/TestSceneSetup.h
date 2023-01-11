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
        AsyncLoader* asyncLoader);

    void setup(
        ShaderRegistry* shaderRegistry,
        NodeRegistry* nodeRegistry,
        MeshTypeRegistry* typeRegistry,
        MaterialRegistry* materialRegistry,
        ModelRegistry* modelRegistry);

private:
    void setupEffectExplosion();

    void setupViewport1();
private:
    const Assets& m_assets;

    AsyncLoader* m_asyncLoader;

    ShaderRegistry* m_shaderRegistry{ nullptr };
    NodeRegistry* m_nodeRegistry{ nullptr };
    MeshTypeRegistry* m_typeRegistry{ nullptr };
    MaterialRegistry* m_materialRegistry{ nullptr };
    ModelRegistry* m_modelRegistry{ nullptr };
};
