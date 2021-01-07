#include "Node.h"

Node::Node(ModelMesh* mesh, const glm::vec3& pos) : mesh(mesh), pos(pos)
{
	dirtyMat = true;
}

Node::~Node()
{
}

int Node::draw(const RenderContext& ctx)
{
	updateModelMatrix();

	mesh->bind(ctx);

	std::string transformName("transform");
	mesh->shader->setMat4(transformName, ctx.projected * modelMat);

	std::string modelName("model");
	mesh->shader->setMat4(modelName, modelMat);

	std::string normalName("normalMat");
	mesh->shader->setMat3(normalName, normalMat);

	mesh->draw(ctx);
	return 0;
}

void Node::updateModelMatrix() {
	if (!dirtyMat) {
		return;
	}

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
	glm::mat3 normalMat = glm::transpose(glm::inverse(modelMat));
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
	return 0.0f;
}
