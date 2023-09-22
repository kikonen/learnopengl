#include "Particle.h"


//Material Particle::getMaterial(
//    const Assets& assets,
//    std::string_view path,
//    std::string_view normalMapPath)
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
//    std::string_view path,
//    std::string_view normalMapPath)
//{
//    auto type = std::make_shared<MeshType>(MeshType::nextID(), Program::getProgram(assets, TEX_TEXTURE, ""));
//    type->renderBack = true;
//
//    QuadMesh* mesh = new QuadMesh(path);
//    mesh->material = Sprite::getMaterial(assets, path, normalMapPath);
//
//    return type;
//}
