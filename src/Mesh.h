#pragma once

#include "MeshBuffers.h"
#include "Assets.h"
#include "RenderContext.h"

class Mesh
{
public:
	Mesh();
	~Mesh();

	virtual void prepare(const Assets& assets) = 0;
	virtual void prepareBuffers(MeshBuffers& curr) = 0;
	virtual void bind(const RenderContext& ctx, Shader* shader) = 0;
	virtual void draw(const RenderContext& ctx) = 0;
	virtual void drawInstanced(const RenderContext& ctx, int instanceCount) = 0;

public:
	MeshBuffers buffers;
};

