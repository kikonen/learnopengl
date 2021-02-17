#include "Sprite.h"

#include "MeshLoader.h"
#include "QuadMesh.h"

ModelMesh* spriteMesh = nullptr;


Mesh* Sprite::getMesh(const Assets& assets, Material* material)
{
	QuadMesh* mesh = new QuadMesh("sprite");
	mesh->material = material;

	return mesh;
}

Material* Sprite::getMaterial(const Assets& assets, const std::string& name)
{
	Material* material = new Material(name);
	material->ns = 100;
	material->ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
	material->map_kd = name;
	material->loadTextures(assets.spritesDir + "/");
	material->diffuseTex->unitIndex = 0;

	return material;
}

NodeType* Sprite::getNodeType(const Assets& assets, const std::string& name)
{
	NodeType* type = new NodeType(NodeType::nextID(), Shader::getShader(assets, TEX_TEXTURE, ""));
	type->renderBack = true;

	Material* material = Sprite::getMaterial(assets, "Skeleton_VH.PNG");
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
