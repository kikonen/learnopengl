#include "Node.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/GL.h"

#include "controller/NodeController.h";


Node::Node(NodeType* type)
	: type(type)
{
}

Node::~Node()
{
}

void Node::prepare(const Assets& assets)
{
	if (controller) {
		controller->prepare(*this);
	}
}

bool Node::update(const RenderContext& ctx)
{
	if (!controller) return false;
	return controller->update(ctx, *this);
}

void Node::bind(const RenderContext& ctx, Shader* shader)
{
	updateModelMatrix();
}

void Node::bindBatch(const RenderContext& ctx, Batch& batch)
{
	updateModelMatrix();
	batch.modelMatrices.push_back(modelMat);
	batch.normalMatrices.push_back(normalMat);
}

void Node::draw(const RenderContext& ctx)
{
	// NOTE KI shader side supports *ONLY* instanced rendering
	singleBatch.size = 1;
	singleBatch.prepare(type);

	singleBatch.draw(ctx, this, type->boundShader);
	//type->mesh->draw(ctx);
}

void Node::updateModelMatrix() {
	if (!dirtyMat) {
		return;
	}
	dirtyMat = false;

	// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
	glm::mat4 rotMat = glm::toMat4(glm::quat(glm::radians(rotation)));

	glm::mat4 transMat = glm::translate(
		glm::mat4(1.0f),
		pos
	);

	glm::mat4 scaleMat = glm::scale(
		glm::mat4(1.0f),
		scale
	);

	modelMat = transMat * rotMat * scaleMat;

	// https://learnopengl.com/Lighting/Basic-Lighting
	// http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
	// normal = mat3(transpose(inverse(model))) * aNormal;
	normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
}

void Node::setPos(const glm::vec3& pos) {
	this->pos = pos;
	dirtyMat = true;
}

const glm::vec3&  Node::getPos() {
	return pos;
}

void Node::setRotation(const glm::vec3& rotation) {
	this->rotation = rotation;
	dirtyMat = true;
}

const glm::vec3&  Node::getRotation() {
	return rotation;
}

void Node::setScale(float scale) {
	this->scale.x = scale;
	this->scale.y = scale;
	this->scale.z = scale;
	dirtyMat = true;
}

void Node::setScale(const glm::vec3& scale)
{
	this->scale = scale;
	dirtyMat = true;
}

const glm::vec3& Node::getScale() {
	return scale;
}
