#include "Material.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


Material::Material(std::string& name)
{
	this->name = name;
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

	texture = new Texture(texturePath);
	texture->load();

	return 0;
}

void Material::prepare(Shader* shader)
{
	if (texture) {
		texture->prepare(shader);
	}
}

void Material::bind()
{
	if (texture) {
		texture->bind();
	}
}
