#include "Material.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

#include "ImageTexture.h"

constexpr int DIFFUSE_IDX = 0;
constexpr int EMISSION_IDX = 1;
constexpr int SPECULAR_IDX = 2;
constexpr int NORMAL_MAP_IDX = 3;
constexpr int DUDV_MAP_IDX = 4;


std::shared_ptr<Material> createGoldMaterial() {
	std::shared_ptr<Material> mat = std::make_shared<Material>("gold", "");
	mat->ns = 51.2f;
	mat->ks = glm::vec4(0.6283f, 0.5559f, 0.3661f, 1.f);
	mat->ka = glm::vec4(0.2473f, 0.1995f, 0.0745f, 1.f);
	mat->kd = glm::vec4(0.7516f, 0.6065f, 0.2265f, 1.f);
	return mat;
}

std::shared_ptr<Material> createSilverMaterial() {
	std::shared_ptr<Material> mat = std::make_shared<Material>("silver", "");
	mat->ns = 51.2f;
	mat->ks = glm::vec4(0.5083f, 0.5083f, 0.5083f, 1.f);
	mat->ka = glm::vec4(0.1923f, 0.1923f, 0.1923f, 1.f);
	mat->kd = glm::vec4(0.5075f, 0.5075f, 0.5075f, 1.f);
	return mat;
}

std::shared_ptr<Material> createBronzeMaterial() {
	std::shared_ptr<Material> mat = std::make_shared<Material>("bronze", "");
	mat->ns = 25.6f;
	mat->ks = glm::vec4(0.3936f, 0.2719f, 0.1667f, 1.f);
	mat->ka = glm::vec4(0.2125f, 0.1275f, 0.0540f, 1.f);
	mat->kd = glm::vec4(0.7140f, 0.4284f, 0.1814f, 1.f);
	return mat;
}

std::shared_ptr<Material> Material::createDefaultMaterial() {
	std::shared_ptr<Material> mat = std::make_shared<Material>("default", "");
	mat->ns = 100.f;
	mat->ks = glm::vec4(0.9f, 0.9f, 0.0f, 1.f);
	mat->ka = glm::vec4(0.3f, 0.3f, 0.0f, 1.f);
	mat->kd = glm::vec4(0.8f, 0.8f, 0.0f, 1.f);
	return mat;
}

std::shared_ptr<Material> Material::createMaterial(MaterialType type)
{
	switch (type) {
	case MaterialType::gold: return createGoldMaterial();
	case MaterialType::silver: return createSilverMaterial();
	case MaterialType::bronze: return createBronzeMaterial();
	}

	return createDefaultMaterial();
}

Material::Material(const std::string& name, const std::string& baseDir)
	: name(name), baseDir(baseDir)
{
}

Material::~Material()
{
	KI_INFO_SB("MATERIAL: " << name << " delete");
	materialIndex = -2;
}

void Material::loadTextures()
{
	if (loaded) return;
	loaded = true;

	textures.reserve(5);
	loadTexture(DIFFUSE_IDX, baseDir, map_kd);
	loadTexture(EMISSION_IDX, baseDir, map_ke);
	loadTexture(SPECULAR_IDX, baseDir, map_ks);
	loadTexture(NORMAL_MAP_IDX, baseDir, map_bump);
	loadTexture(DUDV_MAP_IDX, baseDir, map_dudv);
}

void Material::loadTexture(int idx, const std::string& baseDir, const std::string& name)
{
	BoundTexture tex;

	if (!name.empty()) {

		const char& ch = baseDir.at(baseDir.length() - 1);
		std::string texturePath = (ch == u'/' ? baseDir : baseDir + "/") + name;

		KI_INFO_SB("TEXTURE: " << texturePath);

		auto texture = ImageTexture::getTexture(texturePath, textureSpec);
		if (texture->isValid()) {
			tex.texture = texture;
		}
	}

	textures.emplace_back(tex);
}

void Material::prepare()
{
	unsigned int unitIndex = 0;
	for (auto & x : textures) {
		if (!x.texture) continue;
		if (x.unitIndex == -1) {
			x.unitIndex = unitIndex++;
		}
		x.texture->prepare();
	}
}

void Material::bindArray(Shader* shader, int index, bool bindTextureIDs)
{
	if (textures.empty()) return;

	{
		auto& tex = textures[DIFFUSE_IDX];
		if (tex.texture) {
			shader->textures[tex.unitIndex].set(tex.unitIndex);
		}
	}

	{
		auto& tex = textures[EMISSION_IDX];
		if (tex.texture) {
			shader->textures[tex.unitIndex].set(tex.unitIndex);
		}
	}
	{
		auto& tex = textures[DIFFUSE_IDX];
		if (tex.texture) {
			shader->textures[tex.unitIndex].set(tex.unitIndex);
		}
	}
	{
		auto& tex = textures[DIFFUSE_IDX];
		if (tex.texture) {
			shader->textures[tex.unitIndex].set(tex.unitIndex);
		}
	}
	{
		auto& tex = textures[DUDV_MAP_IDX];
		if (tex.texture) {
			shader->textures[tex.unitIndex].set(tex.unitIndex);
		}
	}

	if (bindTextureIDs) {
		for (auto& x : textures) {
			x.bind();
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

		textures[DIFFUSE_IDX].unitIndex,
		textures[EMISSION_IDX].unitIndex,
		textures[SPECULAR_IDX].unitIndex,
		textures[NORMAL_MAP_IDX].unitIndex,
		textures[DUDV_MAP_IDX].unitIndex,

		pattern,

		reflection,
		refraction,
		refractionRatio,

		fogRatio,
		tiling,
	};
}
