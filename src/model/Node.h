#pragma once

#include <glm/glm.hpp>

#include "component/ParticleGenerator.h"
#include "scene/NodeType.h"
#include "scene/RenderContext.h"
#include "scene/Batch.h"

class NodeController;
class Camera;
class Light;

class Node
{
public:
	Node(NodeType* type);
	virtual ~Node();

	virtual void prepare(const Assets& assets);

	virtual bool update(const RenderContext& ctx);
	virtual void bind(const RenderContext& ctx, Shader* shader);
	virtual void bindBatch(const RenderContext& ctx, Batch& batch);
	virtual void draw(const RenderContext& ctx);

	void setPos(const glm::vec3& pos);
	const glm::vec3& getPos();

	void setRotation(const glm::vec3& rotation);
	const glm::vec3& getRotation();

	void setScale(float scale);
	void setScale(const glm::vec3& scale);
	const glm::vec3& getScale();

	static int nextID();

protected:
	virtual void updateModelMatrix();

public:
	int objectID = 0;

	NodeType* type = nullptr;

	bool selected = false;

	NodeController* controller = nullptr;

	Camera* camera = nullptr;
	Light* light = nullptr;
	ParticleGenerator* particleGenerator = nullptr;

private:
	glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };

	glm::vec3 scale = { 1.f, 1.f, 1.f };

	glm::mat4 modelMat = glm::mat4(1.0f);
	glm::mat3 normalMat = glm::mat3(1.0f);

	bool dirtyMat = true;

	Batch singleBatch;
};

