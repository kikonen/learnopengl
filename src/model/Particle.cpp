#include "Particle.h"


//Material Particle::getMaterial(
//    const Assets& assets,
//    const std::string& path,
//    const std::string& normalMapPath)
//{
//    Material material;
//    material.name = path;
//    material->ns = 100;
//    material->ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
//    material->map_kd = path;
//    material->map_bump = normalMapPath;
//    material->loadTextures(assets.spritesDir + "/");
//
//    return material;
//}
//
//MeshType* Particle::getMeshType(
//    const Assets& assets,
//    const std::string& path,
//    const std::string& normalMapPath)
//{
//    auto type = std::make_shared<MeshType>(MeshType::nextID(), Shader::getShader(assets, TEX_TEXTURE, ""));
//    type->renderBack = true;
//
//    QuadMesh* mesh = new QuadMesh(path);
//    mesh->material = Sprite::getMaterial(assets, path, normalMapPath);
//
//    return type;
//}
