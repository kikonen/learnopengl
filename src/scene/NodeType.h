#pragma once

#include "asset/Mesh.h"
#include "asset/Shader.h"
#include "scene/RenderContext.h"
#include "scene/Batch.h"

class NodeType final
{
public:
	static int nextID();

	NodeType(int typeID, Shader* defaultShader = nullptr);
	~NodeType();

	bool hasReflection();
	bool hasRefraction();

	Material* findMaterial(std::function<bool(Material&)> fn);
	void modifyMaterials(std::function<void(Material&)> fn);

	void prepare(const Assets& assets);
	Shader* bind(const RenderContext& ctx, Shader* shader);
	void unbind(const RenderContext& ctx);

public:
	const int typeID;

	bool blend = false;
	bool light = false;
	bool mirror = false;
	bool water = false;
	bool renderBack = false;
	bool noShadow = false;
	bool batchMode = true;
	bool wireframe = false;

	glm::vec4 mirrorPlane;

	Mesh* mesh = nullptr;
	Shader* defaultShader = nullptr;
	Shader* boundShader = nullptr;

	Batch batch;
private:
};


