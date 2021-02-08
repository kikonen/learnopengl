#pragma once

#include <glm/glm.hpp>

#include "NodeType.h"
#include "RenderContext.h"
#include "Batch.h"

class NodeUpdater;

class Node
{
public:
	Node(NodeType* type);
	~Node();

	virtual void prepare(const Assets& assets);

	virtual bool update(const RenderContext& ctx);
	virtual Shader* bind(const RenderContext& ctx, Shader* shader);
	virtual void bindBatch(const RenderContext& ctx, Batch& batch);
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
	NodeType* type;

	bool selected = false;

	NodeUpdater* updater = nullptr;

private:
	glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
	float scale = 1.0f;

	glm::mat4 modelMat = glm::mat4(1.0f);
	glm::mat3 normalMat = glm::mat3(1.0f);

	bool dirtyMat = true;
};

