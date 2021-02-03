#include "Sprite.h"

#include "MeshLoader.h"

Sprite::Sprite(glm::vec2 size, Material* material)
	: size(size),
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

	MeshLoader loader(Shader::getShader(assets, TEX_TEXTURE), "brickwall2");
	loader.defaultMaterial = material;
	loader.overrideMaterials = true;
	mesh = loader.load();

	float vertices[] = {
		// pos      // tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	blend = true;
	renderBack = true;

	Node::prepare(assets);
}
