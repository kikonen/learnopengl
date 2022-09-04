#include "Particle.h"


//std::shared_ptr<Material> Particle::getMaterial(
//	const Assets& assets,
//	const std::string& path,
//	const std::string& normalMapPath)
//{
//	std::shared_ptr<Material> material = new Material(path);
//	material->ns = 100;
//	material->ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
//	material->map_kd = path;
//	material->map_bump = normalMapPath;
//	material->loadTextures(assets.spritesDir + "/");
//
//	return material;
//}
//
//NodeType* Particle::getNodeType(
//	const Assets& assets,
//	const std::string& path,
//	const std::string& normalMapPath)
//{
//	NodeType* type = new NodeType(NodeType::nextID(), Shader::getShader(assets, TEX_TEXTURE, ""));
//	type->renderBack = true;
//
//	QuadMesh* mesh = new QuadMesh(path);
//	mesh->material = Sprite::getMaterial(assets, path, normalMapPath);
//
//	return type;
//}
