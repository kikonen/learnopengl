#include "Node.h"

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

Shader* Node::bind(const RenderContext& ctx, Shader* shader)
{
	updateModelMatrix();

	if (!shader) {
		return nullptr;
	}

	shader->modelMatrix.set(modelMat);
	shader->normalMatrix.set(normalMat);

	return shader;
}

void Node::bindBatch(const RenderContext& ctx, Batch& batch)
{
	updateModelMatrix();
	batch.matrices.push_back(modelMat);
}

void Node::draw(const RenderContext& ctx)
{
	type->mesh->draw(ctx);
}

void Node::updateModelMatrix() {
	if (!dirtyMat) {
		return;
	}
	dirtyMat = false;

	// ORDER: yaw - pitch - roll
	glm::mat4 rotMat = glm::mat4(1.0f);
	{
		if (rotation.y) {
			rotMat = glm::rotate(
				rotMat,
				glm::radians(rotation.y),
				glm::vec3(0.0f, 1.0f, 0.0f)
			);
		}

		if (rotation.x) {
			rotMat = glm::rotate(
				rotMat,
				glm::radians(rotation.x),
				glm::vec3(1.0f, 0.0f, 0.0f)
			);
		}

		if (rotation.z) {
			rotMat = glm::rotate(
				rotMat,
				glm::radians(rotation.z),
				glm::vec3(0.0f, 0.0f, 1.0f)
			);
		}
	}

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
