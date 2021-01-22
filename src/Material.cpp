#include "Material.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

struct Names {
	int index;
	std::string ambient;
	std::string diffuse;
	std::string specular;
	std::string shininess;

	std::string hasDiffuseTex;
	std::string hasEmissionTex;
	std::string hasSpecularTex;
	std::string hasNormalMap;

	std::string diffuseTex;
	std::string emissionTex;
	std::string specularTex;
	std::string normalMap;
};

std::map<int, const Names*> varNames;

const Names* createNames(const std::string arr, int idx) {
	Names* names = new Names();
	names->index = idx;

	char name[255];

	sprintf_s(name, "%s[%i].ambient", arr.c_str(), idx);
	names->ambient = name;

	sprintf_s(name, "%s[%i].diffuse", arr.c_str(), idx);
	names->diffuse = name;
	
	sprintf_s(name, "%s[%i].specular", arr.c_str(), idx);
	names->specular = name;

	sprintf_s(name, "%s[%i].shininess", arr.c_str(), idx);
	names->shininess = name;
	
	sprintf_s(name, "%s[%i].hasDiffuseTex", arr.c_str(), idx);
	names->hasDiffuseTex = name;
	
	sprintf_s(name, "%s[%i].hasEmissionTex", arr.c_str(), idx);
	names->hasEmissionTex = name;
	
	sprintf_s(name, "%s[%i].hasSpecularTex", arr.c_str(), idx);
	names->hasSpecularTex = name;

	sprintf_s(name, "%s[%i].hasNormalMap", arr.c_str(), idx);
	names->hasNormalMap = name;

	sprintf_s(name, "textures[%i].diffuse", idx);
	names->diffuseTex = name;

	sprintf_s(name, "textures[%i].emission", idx);
	names->emissionTex = name;

	sprintf_s(name, "textures[%i].specular", idx);
	names->specularTex = name;

	sprintf_s(name, "textures[%i].normalMap", idx);
	names->normalMap = name;

	return names;
}

const Names* getNames(int index) {
	if (varNames.count(index)) {
		return varNames[index];
	}

	const Names* names = createNames("materials", index);
	varNames[index] = names;

	return varNames[index];
}

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
	delete normalMap;
}

int Material::loadTextures(const std::string& baseDir)
{
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
	Texture* texture = new Texture(texturePath, normalMap);

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

void Material::bind(Shader* shader, int index, bool useTexture)
{
	const Names* names = getNames(index);

	shader->setVec4(names->ambient, ka);
	shader->setVec4(names->diffuse, kd);
	shader->setVec4(names->specular, ks);
	shader->setFloat(names->shininess, ns);

	if (diffuseTex && useTexture) {
		shader->setInt(names->diffuseTex, diffuseTex->textureIndex);
	}
	if (emissionTex && useTexture) {
		shader->setInt(names->emissionTex, emissionTex->textureIndex);
	}
	if (specularTex && useTexture) {
		shader->setInt(names->specularTex, specularTex->textureIndex);
	}
	if (normalMap) {
		shader->setInt(names->normalMap, normalMap->textureIndex);
	}
	shader->setBool(names->hasDiffuseTex, !!diffuseTex && useTexture);
	shader->setBool(names->hasEmissionTex, !!emissionTex && useTexture);
	shader->setBool(names->hasSpecularTex, !!specularTex && useTexture);
	shader->setBool(names->hasNormalMap, !!normalMap);

	for (auto const x : textures) {
		x->bind(shader);
	}
}
