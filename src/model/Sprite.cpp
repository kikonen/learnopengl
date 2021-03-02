#include "Sprite.h"

#include "asset/QuadMesh.h"


Material* Sprite::getMaterial(
	const Assets& assets, 
	const std::string& path,
	const std::string& normalMapPath)
{
	Material* material = new Material(path, assets.spritesDir + "/");
	material->ns = 100;
	material->ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
	material->map_kd = path;
	material->map_bump = normalMapPath;
	material->loadTextures();

	return material;
}

NodeType* Sprite::getNodeType(
	const Assets& assets, 
	const std::string& path,
	const std::string& normalMapPath)
{
	NodeType* type = new NodeType(NodeType::nextID(), Shader::getShader(assets, TEX_TEXTURE, ""));
	type->renderBack = true;

	QuadMesh* mesh = new QuadMesh(path);
	mesh->material = Sprite::getMaterial(assets, path, normalMapPath);

	type->mesh = mesh;

	return type;
}

Sprite::Sprite(NodeType* type, glm::vec2 size)
	: Node(type),
	size(size)
{
	setScale(glm::vec3(size.x, size.y, 1.f));
}

Sprite::~Sprite()
{
}
