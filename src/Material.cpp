#include "Material.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>


Material* Material::createDefaultMaterial() {
	Material* mat = new Material("default");
	mat->ns = 100.f;
	mat->ks = glm::vec4(0.9f, 0.9f, 0.0f, 1.f);
	mat->ka = glm::vec4(0.3f, 0.3f, 0.0f, 1.f);
	mat->kd = glm::vec4(0.8f, 0.8f, 0.0f, 1.f);
	return mat;
}

Material::Material(const std::string& name)
	: name(name)
{
}

Material::~Material()
{
	delete diffuseTex;
	delete specularTex;
	delete emissionTex;
	delete normalMap;
}

int Material::loadTextures(const std::string& baseDir)
{
	if (loaded) return 0;
	loaded = true;

	diffuseTex = loadTexture(baseDir, map_kd, false);
	emissionTex = loadTexture(baseDir, map_ke, false);
	specularTex = loadTexture(baseDir, map_ks, false);
	normalMap = loadTexture(baseDir, map_bump, true);
	return 0;
}

Texture* Material::loadTexture(const std::string& baseDir, const std::string& name, bool normalMap)
{
	if (name.empty()) {
		return nullptr;
	}

	std::string texturePath = baseDir + name;

	std::cout << "\n== TEXTURE: " << texturePath << " ===\n";

	// NOTE KI sharing fails since causes texture unit conflicts across materials
//	Texture* texture = Texture::getTexture(texturePath, normalMap);
	Texture* texture = new Texture(texturePath, textureMode, normalMap);

	int res = texture->load();

	if (res) {
		delete texture;
		texture = nullptr;
	} else {
		textures.push_back(texture);
	}
	return texture;
}

void Material::prepare()
{
	for (auto const x : textures) {
		x->prepare();
	}
}

void Material::bind(Shader* shader, int index)
{
	TextureInfo info = shader->textures[index];

	if (diffuseTex) {
		info.diffuseTex->set(diffuseTex->textureIndex);
	}
	if (emissionTex) {
		info.emissionTex->set(emissionTex->textureIndex);
	}
	if (specularTex) {
		info.specularTex->set(specularTex->textureIndex);
	}
	if (normalMap) {
		info.normalMap->set(normalMap->textureIndex);
	}

	for (auto const x : textures) {
		x->bind(shader);
	}
}

MaterialUBO Material::toUBO()
{
	return {
		ka,
		kd,
		glm::vec4(0),
		ks,
		ns,

		!!diffuseTex,
		!!emissionTex,
		!!specularTex,
		!!normalMap,

		0,
		0,
		0
	};
}
