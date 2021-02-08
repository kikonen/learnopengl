#pragma once

#include "RenderContext.h"
#include "Mesh.h"
#include "Shader.h"

class NodeType
{
public:
	static int nextID();

	NodeType(int objectID, Shader* defaultShader = nullptr);
	~NodeType();

	void prepare(const Assets& assets);
	Shader* bind(const RenderContext& ctx, Shader* shader);

public:
	const int objectID;

	Mesh* mesh = nullptr;
	Shader* defaultShader = nullptr;
	Shader* boundShader = nullptr;
private:
};

