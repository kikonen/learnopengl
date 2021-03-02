#include "Material.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

#include "ImageTexture.h"


Material* createGoldMaterial() {
	Material* mat = new Material("gold");
	mat->ns = 51.2f;
	mat->ks = glm::vec4(0.6283f, 0.5559f, 0.3661f, 1.f);
	mat->ka = glm::vec4(0.2473f, 0.1995f, 0.0745f, 1.f);
	mat->kd = glm::vec4(0.7516f, 0.6065f, 0.2265f, 1.f);
	return mat;
}

Material* createSilverMaterial() {
	Material* mat = new Material("silver");
	mat->ns = 51.2f;
	mat->ks = glm::vec4(0.5083f, 0.5083f, 0.5083f, 1.f);
	mat->ka = glm::vec4(0.1923f, 0.1923f, 0.1923f, 1.f);
	mat->kd = glm::vec4(0.5075f, 0.5075f, 0.5075f, 1.f);
	return mat;
}

Material* createBronzeMaterial() {
	Material* mat = new Material("bronze");
	mat->ns = 25.6f;
	mat->ks = glm::vec4(0.3936f, 0.2719f, 0.1667f, 1.f);
	mat->ka = glm::vec4(0.2125f, 0.1275f, 0.0540f, 1.f);
	mat->kd = glm::vec4(0.7140f, 0.4284f, 0.1814f, 1.f);
	return mat;
}

Material* Material::createDefaultMaterial() {
	Material* mat = new Material("default");
	mat->ns = 100.f;
	mat->ks = glm::vec4(0.9f, 0.9f, 0.0f, 1.f);
	mat->ka = glm::vec4(0.3f, 0.3f, 0.0f, 1.f);
	mat->kd = glm::vec4(0.8f, 0.8f, 0.0f, 1.f);
	return mat;
}

Material* Material::createMaterial(MaterialType type)
{
	switch (type) {
	case gold: return createGoldMaterial();
	case silver: return createSilverMaterial();
	case bronze: return createBronzeMaterial();
	}

	return createDefaultMaterial();
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
	dudvMapTex = loadTexture(baseDir, map_dudv);
	return 0;
}

BoundTexture* Material::loadTexture(const std::string& baseDir, const std::string& name)
{
	if (name.empty()) {
		return nullptr;
	}

	const char& ch = baseDir.at(baseDir.length() - 1);
	std::string texturePath = (ch == u'/' ? baseDir : baseDir + "/") + name;

	KI_INFO_SB("TEXTURE: " << texturePath);

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

void Material::bindArray(Shader* shader, int index, bool bindTextureIDs)
{
	if (textures.empty()) return;

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
	if (dudvMapTex) {
		shader->textures[dudvMapTex->unitIndex].set(dudvMapTex->unitIndex);
	}

	if (bindTextureIDs) {
		for (auto& x : textures) {
			x->bind();
		}
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
		dudvMapTex ? dudvMapTex->unitIndex : -1,

		pattern,

		reflection,
		refraction,

		fogRatio,
		tiling,
	};
}
