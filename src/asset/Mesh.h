#pragma once

#include <functional>

#include "MeshBuffers.h"
#include "Assets.h"
#include "Material.h"
#include "scene/RenderContext.h"
#include "ki/GL.h"

class Mesh
{
public:
	Mesh();
	virtual ~Mesh();

	virtual bool hasReflection() = 0;
	virtual bool hasRefraction() = 0;

	virtual Material* findMaterial(std::function<bool(Material&)> fn) = 0;
	virtual void modifyMaterials(std::function<void(Material&)> fn) = 0;

	virtual void prepare(const Assets& assets) = 0;
	virtual void prepareBuffers(MeshBuffers& curr) = 0;
	virtual void bind(const RenderContext& ctx, Shader* shader) = 0;
	virtual void draw(const RenderContext& ctx) = 0;
	virtual void drawInstanced(const RenderContext& ctx, int instanceCount) = 0;

public:
	MeshBuffers buffers;
};

