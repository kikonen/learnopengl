#pragma once

#include <glm/glm.hpp>
#include "Mesh.h"
#include "RenderContext.h"

class Node
{
public:
	Node(Mesh* mesh = nullptr);
	~Node();

	virtual void prepare(const Assets& assets);

	virtual void update(const RenderContext& ctx);
	virtual Shader* bind(const RenderContext& ctx, Shader* shader);
	virtual void draw(const RenderContext& ctx);

	void setPos(const glm::vec3& pos);
	const glm::vec3& getPos();

	void setRotation(const glm::vec3& rotation);
	const glm::vec3& getRotation();

	void setScale(float scale);
	float getScale();

protected:
	virtual void updateModelMatrix();

public:
	Mesh* mesh;
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
};

