#include "Node.h"

#include "KIGL.h"
#include "NodeUpdater.h";


Node::Node(NodeType* type)
	: type(type)
{
}

Node::~Node()
{
}

void Node::prepare(const Assets& assets)
{
	if (updater) {
		updater->prepare(*this);
	}
}

bool Node::update(const RenderContext& ctx)
{
	if (!updater) return false;
	return updater->update(ctx, *this);
}

Shader* Node::bind(const RenderContext& ctx, Shader* shader)
{
	updateModelMatrix();

	if (!shader) {
		return nullptr;
	}

	shader->modelMatrix.set(modelMat);
	shader->normalMatrix.set(normalMat);
	shader->drawInstanced.set(false);

	return shader;
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
		rotMat = glm::rotate(
			rotMat,
			glm::radians(rotation.y),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		rotMat = glm::rotate(
			rotMat,
			glm::radians(rotation.x),
			glm::vec3(1.0f, 0.0f, 0.0f)
		);

		rotMat = glm::rotate(
			rotMat,
			glm::radians(rotation.z),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
	}

	glm::mat4 scaleMat = glm::scale(
		glm::mat4(1.0f),
		glm::vec3(scale)
	);

	glm::mat4 transMat = glm::translate(
		glm::mat4(1.0f),
		pos
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
	this->scale = scale;
	dirtyMat = true;
}

float Node::getScale() {
	return scale;
}
