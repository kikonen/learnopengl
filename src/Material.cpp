#include "Material.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


Material::Material(const std::string& name, unsigned int materialIndex)
	: name(name),
	materialIndex(materialIndex)
{
}

Material::~Material()
{
	delete diffuseTex;
	delete specularTex;
	delete emissionTex;
}

int Material::loadTextures(const std::string& baseDir)
{
	diffuseTex = loadTexture(baseDir, map_kd);
	emissionTex = loadTexture(baseDir, map_ke);
	specularTex = loadTexture(baseDir, map_ks);
	return 0;
}

Texture* Material::loadTexture(const std::string& baseDir, const std::string& name)
{
	if (name.empty()) {
		return nullptr;
	}

	std::string texturePath = baseDir + "/" + name;

	std::cout << "\n== TEXTURE: " << texturePath << " ===\n";

	Texture* texture = new Texture(texturePath);
	int res = texture->load();

	if (res) {
		delete texture;
		texture = nullptr;
	} else {
		textures.push_back(texture);
	}
	return texture;
}

void Material::prepare(Shader* shader)
{
	for (auto const x : textures) {
		x->prepare(shader);
	}
}

void Material::bind(Shader* shader)
{
	for (auto const x : textures) {
		x->bind(shader);
	}
}
