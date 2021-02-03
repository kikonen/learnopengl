#pragma once

#include <glm/glm.hpp>
#include "ModelMesh.h"
#include "RenderContext.h"

class Node
{
public:
	Node(ModelMesh* mesh);
	~Node();

	virtual void prepare();

	virtual ShaderInfo* bind(const RenderContext& ctx, Shader* shader);
	virtual void draw(const RenderContext& ctx);
	virtual void drawInstanced(const RenderContext& ctx, int instanceCount);

	void setPos(const glm::vec3& pos);
	const glm::vec3& getPos();

	void setScale(float scale);
	float getScale();

	void setRotation(const glm::vec3& rotation);
	const glm::vec3& getRotation();

public:
	ModelMesh* mesh;
	bool blend = false;
	bool light = false;
	bool renderBack = false;
	bool skipShadow = false;
	bool selected = false;
private:
	
	glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
	float scale = 1.0f;

	glm::mat4 modelMat = glm::mat4(1.0f);
	glm::mat3 normalMat = glm::mat3(1.0f);

	bool prepared = false;
	bool dirtyMat = true;

	void updateModelMatrix();
};

