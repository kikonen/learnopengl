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

class ModelMesh {
public:
	ModelMesh(
		const std::string& modelName);

	ModelMesh(
		const std::string& modelName,
		const std::string& path);

	~ModelMesh();

	void prepare();
	void prepareBuffers(MeshBuffers& curr);
	ShaderInfo* bind(const RenderContext& ctx, Shader* shader);
	void draw(const RenderContext& ctx);
	void drawInstanced(const RenderContext& ctx, int instanceCount);

private:
	ShaderInfo* prepareShader(Shader* shader);
public:
	bool useWireframe = false;

	ShaderInfo* bound = nullptr;

	Shader* defaultShader = nullptr;
	Material* defaultMaterial = nullptr;

	unsigned int textureCount = 0;

	MeshBuffers buffers;

	std::vector<Tri*> tris;
	std::vector<Vertex*> vertexes;

	std::map<std::string, Material*> materials;

	bool hasTexture = false;
private:
	std::map<std::string, ShaderInfo*> shaders;

	const std::string modelName;
	const std::string path;

	MaterialsUBO materialsUbo;
	unsigned int  materialsUboId = -1;
	unsigned int materialsUboSize = -1;
	const bool bindTexture = true;
};
