#pragma once

#include "Node.h"

class AsteroidBeltNode : public Node
{
public:
	AsteroidBeltNode(ModelMesh* mesh);

	ShaderInfo* prepare(Shader* shader) override;
	int bind(const RenderContext& ctx, Shader* shader) override;
	void draw(const RenderContext& ctx) override;

private:
	void setup();

private:
	std::vector<glm::mat4> asteroidMatrixes;
	unsigned int asteroidBuffer = 0;
	bool preparedAsteroids = false;
};

