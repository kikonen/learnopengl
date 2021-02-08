#include "Sprite.h"

#include "MeshLoader.h"


Sprite::Sprite(NodeType* type, glm::vec2 size, Material* material)
	: Node(type),
	size(size),
	material(material)
{
}

Sprite::~Sprite()
{
}

void Sprite::prepare(const Assets& assets)
{
//	mesh = new Mesh("sprite");
//	mesh->materials[material->name] = material;
//	mesh->defaultShader = Shader::getShader(assets, TEX_TEXTURE, "");

	//MeshLoader loader(assets, "brickwall2");
	//loader.defaultMaterial = material;
	//loader.overrideMaterials = true;
	//type->mesh = loader.load();

	float vertices[] = {
		// pos      // tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	renderBack = true;

	Node::prepare(assets);
}
