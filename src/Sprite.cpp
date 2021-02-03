#include "Sprite.h"

Sprite::Sprite(glm::vec2 size, Material* material)
	: size(size),
	material(material)
{
}

Sprite::~Sprite()
{
	delete mesh;
}

void Sprite::prepare()
{
	mesh = new Mesh("sprite");
	mesh->materials[material->name] = material;
//	mesh->defaultShader = Shader::getShader(assets, TEX_TEXTURE, "");

	float vertices[] = {
		// pos      // tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};
}

Shader* Sprite::bind(const RenderContext& ctx, Shader* shader)
{
	return nullptr;
}

void Sprite::draw(const RenderContext& ctx)
{
	//mesh->draw(ctx);
	glBindVertexArray(0);
}

void Sprite::drawInstanced(const RenderContext& ctx, int instanceCount)
{
	//mesh->drawInstanced(ctx, instanceCount);
	glBindVertexArray(0);
}

void Sprite::updateModelMatrix() {
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

void Sprite::setPos(const glm::vec3& pos) {
	this->pos = pos;
	dirtyMat = true;
}

const glm::vec3& Sprite::getPos() {
	return pos;
}

void Sprite::setRotation(const glm::vec3& rotation) {
	this->rotation = rotation;
	dirtyMat = true;
}

const glm::vec3& Sprite::getRotation() {
	return rotation;
}

void Sprite::setScale(float scale) {
	this->scale = scale;
	dirtyMat = true;
}

float Sprite::getScale() {
	return scale;
}
