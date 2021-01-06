#include "Material.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


Material::Material(std::string& name, unsigned int materialId)
	: name(name),
	materialId(materialId)
{
}

Material::~Material()
{
	delete texture;
}

int Material::loadTexture(const std::string& baseDir)
{
	if (map_kd.empty()) {
		return 0;
	}

	std::string texturePath = baseDir + "/" + map_kd;

	std::cout << "\n== TEXTURE: " << texturePath << " ===\n";

	Texture* tmp = new Texture(texturePath);
	int res = tmp->load();
	if (!res) {
		texture = tmp;
	}
	return res;
}

void Material::prepare(Shader* shader)
{
	if (texture) {
		texture->prepare(shader);
	}
}

void Material::bind(Shader* shader)
{
	if (texture) {
		texture->bind(shader);
	}
}
