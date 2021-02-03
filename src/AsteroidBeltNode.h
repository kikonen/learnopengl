#pragma once

#include "Node.h"

class AsteroidBeltNode : public Node
{
public:
	AsteroidBeltNode(ModelMesh* mesh);

	void prepare() override;
	ShaderInfo* bind(const RenderContext& ctx, Shader* shader) override;
	void draw(const RenderContext& ctx) override;

private:
	void setup();

	void prepareBuffer(std::vector<glm::mat4> matrices);

public:
	Node* planet = nullptr;

private:
	std::vector<glm::mat4> asteroidMatrices;
	std::vector<glm::mat4> selectionMatrices;

	MeshBuffers selectedBuffers;

	unsigned int asteroidBuffer = -1;
	unsigned int selectedBuffer = -1;
};

