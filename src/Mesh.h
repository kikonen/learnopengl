#pragma once

#include <string>

#include "Engine.h"
#include "Shader.h"
#include "RenderContext.h"

class Mesh
{
public:
	Mesh(const Engine& engine, const std::string name);
	~Mesh();

	virtual int prepare() = 0;
	virtual int bind(const RenderContext& ctx) = 0;
	virtual int draw(const RenderContext& ctx) = 0;
public:
	unsigned int VBO = 0;
	unsigned int VAO = 0;
	unsigned int EBO = 0;

	std::string name;
	Shader* shader;
protected:
	const Engine& engine;
};

