#pragma once

#include <glm/glm.hpp>
#include "ModelMesh.h"

class Node
{
public:
	Node(ModelMesh* mesh, const glm::vec3& pos);
	~Node();

	int draw(float dt, const glm::mat4& vpMat);

	void setPos(const glm::vec3& pos);
	const glm::vec3& getPos();

	void setScale(float scale);
	float getScale();

	void setRotation(const glm::vec3& rotation);
	const glm::vec3& getRotation();

public:
	ModelMesh* mesh;

private:

	glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
	float scale = 1.0f;

	glm::mat4 modelMat = glm::mat4(1.0f);

	bool dirtyMat = true;

	void updateModelMatrix();
};

