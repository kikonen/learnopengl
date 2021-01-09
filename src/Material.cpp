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
	delete emissiveTex;
}

int Material::loadTextures(const std::string& baseDir)
{
	diffuseTex = loadTexture(baseDir, map_kd);
	emissiveTex = loadTexture(baseDir, map_ke);
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
	}
	return texture;
}

void Material::prepare(Shader* shader)
{
	if (diffuseTex) {
		diffuseTex->prepare(shader);
	}
	if (emissiveTex) {
		emissiveTex->prepare(shader);
	}
	if (specularTex) {
		specularTex->prepare(shader);
	}
}

void Material::bind(Shader* shader)
{
	if (diffuseTex) {
		diffuseTex->bind(shader);
	}
	if (emissiveTex) {
		emissiveTex->bind(shader);
	}
	if (specularTex) {
		specularTex->bind(shader);
	}
}
