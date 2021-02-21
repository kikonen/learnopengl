#include "Sprite.h"

#include "asset/QuadMesh.h"


Mesh* Sprite::getMesh(const Assets& assets, Material* material)
{
	QuadMesh* mesh = new QuadMesh("sprite");
	mesh->material = material;

	return mesh;
}

Material* Sprite::getMaterial(
	const Assets& assets, 
	const std::string& path,
	const std::string& normalMapPath)
{
	Material* material = new Material(path);
	material->ns = 100;
	material->ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
	material->map_kd = path;
	material->map_bump = normalMapPath;
	material->loadTextures(assets.spritesDir + "/");

	return material;
}

NodeType* Sprite::getNodeType(
	const Assets& assets, 
	const std::string& path,
	const std::string& normalMapPath)
{
	NodeType* type = new NodeType(NodeType::nextID(), Shader::getShader(assets, TEX_TEXTURE, ""));
	type->renderBack = true;

	Material* material = Sprite::getMaterial(assets, path, normalMapPath);
	type->mesh = getMesh(assets, material);

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
