#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "Assets.h"
#include "Shader.h"
#include "ShaderInfo.h"
#include "RenderContext.h"

#include "Tri.h"
#include "Material.h"
#include "Shader.h"
#include "Camera.h"
#include "Vertex.h"
#include "Light.h"
#include "Assets.h"
#include "MeshBuffers.h"

class Mesh {
public:
	Mesh(
		const std::string& modelName);

	Mesh(
		const std::string& modelName,
		const std::string& path);

	~Mesh();

	void prepare();
	void prepareBuffers(MeshBuffers& curr);
	ShaderInfo* bind(const RenderContext& ctx, Shader* shader);
	void draw(const RenderContext& ctx);
	void drawInstanced(const RenderContext& ctx, int instanceCount);

private:
	ShaderInfo* prepareShader(Shader* shader);
public:
	ShaderInfo* bound = nullptr;

	Shader* defaultShader = nullptr;
	Material* defaultMaterial = nullptr;

	MeshBuffers buffers;

	std::vector<Tri*> tris;
	std::vector<Vertex*> vertices;

	std::map<std::string, Material*> materials;

private:
	std::map<std::string, ShaderInfo*> shaders;

	const std::string modelName;
	const std::string path;

	MaterialsUBO materialsUbo;
	unsigned int  materialsUboId = -1;
	unsigned int materialsUboSize = -1;
};
