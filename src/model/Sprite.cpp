#include "Sprite.h"

#include "asset/QuadMesh.h"


std::shared_ptr<Material> Sprite::getMaterial(
    const Assets& assets, 
    const std::string& path,
    const std::string& normalMapPath)
{
    std::shared_ptr<Material> material = std::make_shared<Material>(path, assets.spritesDir + "/");
    material->ns = 100;
    material->ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
    material->map_kd = path;
    material->map_bump = normalMapPath;
    material->loadTextures(assets);

    return material;
}

std::shared_ptr<NodeType> Sprite::getNodeType(
    const Assets& assets, 
    ShaderRegistry& shaders,
    const std::string& path,
    const std::string& normalMapPath)
{
    auto type = std::make_shared<NodeType>(NodeType::nextID(), shaders.getShader(assets, TEX_TEXTURE, { DEF_USE_ALPHA }));
    type->renderBack = true;

    auto quad = std::make_unique<QuadMesh>(path);
    quad->material = Sprite::getMaterial(assets, path, normalMapPath);

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
