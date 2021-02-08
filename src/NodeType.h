#pragma once

#include "RenderContext.h"
#include "Mesh.h"
#include "Shader.h"
#include "Batch.h"

class NodeType
{
public:
	static int nextID();

	NodeType(int objectID, Shader* defaultShader = nullptr);
	~NodeType();

	void prepare(const Assets& assets);
	Shader* bind(const RenderContext& ctx, Shader* shader);

	//bool operator< (const NodeType& b) {
	//	return objectID < b.objectID;
	//}

public:
	const int objectID;

	bool blend = false;
	bool light = false;
	bool renderBack = false;
	bool skipShadow = false;

	Mesh* mesh = nullptr;
	Shader* defaultShader = nullptr;
	Shader* boundShader = nullptr;

	Batch batch;
private:
};


