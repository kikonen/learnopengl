#include "Sprite.h"

#include "asset/QuadMesh.h"


Material Sprite::createMaterial(
    const Assets& assets, 
    const std::string& path,
    const std::string& normalMapPath)
{
    Material material;
    material.name = path;
    material.type = MaterialType::sprite;
    material.ns = 100;
    material.ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
    material.map_kd = path;
    material.map_bump = normalMapPath;
    material.loadTextures(assets);

    return material;
}

std::shared_ptr<NodeType> Sprite::getNodeType(
    const Assets& assets, 
    ShaderRegistry& shaders,
    const std::string& path,
    const std::string& normalMapPath)
{
    auto type = std::make_shared<NodeType>(
        NodeType::nextID(),
        shaders.getShader(assets, TEX_TEXTURE, { DEF_USE_ALPHA }));

    // TODO KI add "solid with holes" mode; sprite does not neat *BLEND*, but *ALPHA*
    // => Two different things
    type->flags.blend = true;
    type->flags.renderBack = true;

    auto quad = std::make_unique<QuadMesh>(path);
    quad->material = Sprite::createMaterial(assets, path, normalMapPath);

    type->mesh = std::unique_ptr<Mesh>(std::move(quad));

    return type;
}

Sprite::Sprite(std::shared_ptr<NodeType> type, glm::vec2 size)
    : Node(type),
    size(size)
{
    setScale(glm::vec3(size.x, size.y, 1.f));
}

Sprite::~Sprite()
{
}
