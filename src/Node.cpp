#include "Node.h"

#include "KIGL.h"

Node::Node(ModelMesh* mesh) : mesh(mesh)
{
	dirtyMat = true;
}

Node::~Node()
{
}

ShaderInfo* Node::prepare(Shader* shader)
{
	ShaderInfo* info = mesh->prepare(shader);
	return info;
}

int Node::bind(const RenderContext& ctx, Shader* shader)
{
	updateModelMatrix();

	ShaderInfo* info = mesh->bind(ctx, shader);
	if (!info) {
		return -1;
	}

	if (flat) {
		glDisable(GL_CULL_FACE);
	} else {
		glEnable(GL_CULL_FACE);
	}

	shader = info->shader;

//	shader->setMat4("transform", ctx.projected * modelMat);
//	shader->setMat4("projected", ctx.projected);
	shader->setMat4("model", modelMat);
	shader->setMat3("normalMat", normalMat);
	shader->setBool("drawInstanced", false);

	return 0;
}

void Node::draw(const RenderContext& ctx)
{
	mesh->draw(ctx);
	glBindVertexArray(0);
}

void Node::drawInstanced(const RenderContext& ctx, int instanceCount)
{
	mesh->drawInstanced(ctx, instanceCount);
	glBindVertexArray(0);
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
