#pragma once

#include "asset/Mesh.h"
#include "asset/Shader.h"
#include "scene/RenderContext.h"
#include "scene/Batch.h"

class NodeType final
{
public:
	static int nextID();

	NodeType(int objectID, Shader* defaultShader = nullptr);
	~NodeType();

	bool hasReflection();
	bool hasRefraction();

	void setReflection(float reflection);
	void setRefraction(float refraction);

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
	bool noShadow = false;
	bool batchMode = true;

	Mesh* mesh = nullptr;
	Shader* defaultShader = nullptr;
	Shader* boundShader = nullptr;

	Batch batch;
private:
};


