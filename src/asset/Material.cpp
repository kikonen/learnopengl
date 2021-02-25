#include "Material.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

#include "ImageTexture.h"


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
	//delete diffuseTex;
	//delete specularTex;
	//delete emissionTex;
	//delete normalMap;
}

int Material::loadTextures(const std::string& baseDir)
{
	if (loaded) return 0;
	loaded = true;

	textures.reserve(2);
	diffuseTex = loadTexture(baseDir, map_kd);
	emissionTex = loadTexture(baseDir, map_ke);
	specularTex = loadTexture(baseDir, map_ks);
	normalMapTex = loadTexture(baseDir, map_bump);
	return 0;
}

BoundTexture* Material::loadTexture(const std::string& baseDir, const std::string& name)
{
	if (name.empty()) {
		return nullptr;
	}

	std::string texturePath = baseDir + name;

	std::cout << "\n== TEXTURE: " << texturePath << " ===\n";

	ImageTexture* texture = ImageTexture::getTexture(texturePath, textureSpec);
	if (!texture) return nullptr;

	BoundTexture* tex = new BoundTexture();
	tex->texture = texture;

 	textures.push_back(tex);

	return tex;
}

void Material::prepare()
{
	unsigned int unitIndex = 0;
	for (auto & x : textures) {
		if (x->unitIndex == -1) {
			x->unitIndex = unitIndex++;
		}
		x->texture->prepare();
	}
}

//void Material::bind(Shader* shader)
//{
//	TextureInfo& info = *shader->texture;
//
//	if (diffuseTex) {
//		info.diffuseTex->set(diffuseTex->unitIndex);
//	}
//	if (emissionTex) {
//		info.emissionTex->set(emissionTex->unitIndex);
//	}
//	if (specularTex) {
//		info.specularTex->set(specularTex->unitIndex);
//	}
//	if (normalMapTex) {
//		info.normalMap->set(normalMapTex->unitIndex);
//	}
//
//	for (auto & x : textures) {
//		x->bind();
//	}
//}

void Material::bindArray(Shader* shader, int index)
{
	if (diffuseTex) {
		shader->textures[diffuseTex->unitIndex].set(diffuseTex->unitIndex);
	}
	if (emissionTex) {
		shader->textures[emissionTex->unitIndex].set(emissionTex->unitIndex);
	}
	if (specularTex) {
		shader->textures[specularTex->unitIndex].set(specularTex->unitIndex);
	}
	if (normalMapTex) {
		shader->textures[normalMapTex->unitIndex].set(normalMapTex->unitIndex);
	}

	for (auto& x : textures) {
		x->bind();
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

		diffuseTex ? diffuseTex->unitIndex : -1,
		emissionTex ? emissionTex->unitIndex : -1,
		specularTex ? specularTex->unitIndex : -1,
		normalMapTex ? normalMapTex->unitIndex : -1,

		0,
		0,
		0
	};
}
