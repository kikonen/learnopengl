#include "Batch.h"

#include "model/Node.h"
#include "NodeType.h"

Batch::Batch()
{
}

void Batch::prepare(NodeType* type)
{
	if (size == 0) return;
	if (prepared) return;
	prepared = true;

	KI_GL_CALL(glBindVertexArray(type->mesh->buffers.VAO));

	// model
	{
		if (modelMatrices.empty()) {
			modelMatrices.reserve(size);
			glm::mat4 tmp(0.f);
			for (unsigned int i = 0; i < size; i++) {
				modelMatrices.emplace_back(tmp);
			}
		}
		else {
			size = modelMatrices.size();
		}

		glGenBuffers(1, &modelBuffer);
		KI_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, modelBuffer));
		KI_GL_CALL(glBufferData(GL_ARRAY_BUFFER, size * sizeof(glm::mat4), &modelMatrices[0], GL_DYNAMIC_DRAW));
		//glCreateBuffers(1, &modelBuffer);
		//KI_GL_CALL(glNamedBufferStorage(modelBuffer, size * sizeof(glm::mat4), &modelMatrices[0], GL_DYNAMIC_STORAGE_BIT));

		// NOTE mat4 as vertex attributes *REQUIRES* hacky looking approach
		size_t vecSize = sizeof(glm::vec4);

		glVertexAttribPointer(ATTR_INSTANCE_MODEL_MATRIX_1, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, (void*)0);
		glVertexAttribPointer(ATTR_INSTANCE_MODEL_MATRIX_2, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, (void*)(1 * vecSize));
		glVertexAttribPointer(ATTR_INSTANCE_MODEL_MATRIX_3, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, (void*)(2 * vecSize));
		glVertexAttribPointer(ATTR_INSTANCE_MODEL_MATRIX_4, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, (void*)(3 * vecSize));

		glVertexAttribDivisor(ATTR_INSTANCE_MODEL_MATRIX_1, 1);
		glVertexAttribDivisor(ATTR_INSTANCE_MODEL_MATRIX_2, 1);
		glVertexAttribDivisor(ATTR_INSTANCE_MODEL_MATRIX_3, 1);
		glVertexAttribDivisor(ATTR_INSTANCE_MODEL_MATRIX_4, 1);

		glEnableVertexAttribArray(ATTR_INSTANCE_MODEL_MATRIX_1);
		glEnableVertexAttribArray(ATTR_INSTANCE_MODEL_MATRIX_2);
		glEnableVertexAttribArray(ATTR_INSTANCE_MODEL_MATRIX_3);
		glEnableVertexAttribArray(ATTR_INSTANCE_MODEL_MATRIX_4);
	}

	// normal
	{
		if (normalMatrices.empty()) {
			normalMatrices.reserve(size);
			glm::mat3 tmp(0.f);
			for (unsigned int i = 0; i < size; i++) {
				normalMatrices.emplace_back(tmp);
			}
		}
		else {
			size = normalMatrices.size();
		}

		glGenBuffers(1, &normalBuffer);
		KI_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, normalBuffer));
		KI_GL_CALL(glBufferData(GL_ARRAY_BUFFER, size * sizeof(glm::mat3), &normalMatrices[0], GL_DYNAMIC_DRAW));
		//glCreateBuffers(1, &normalBuffer);
		//KI_GL_CALL(glNamedBufferStorage(normalBuffer, size * sizeof(glm::mat3), &normalMatrices[0], GL_DYNAMIC_STORAGE_BIT));

		// NOTE mat3 as vertex attributes *REQUIRES* hacky looking approach
		size_t vecSize = sizeof(glm::vec3);

		glVertexAttribPointer(ATTR_INSTANCE_NORMAL_MATRIX_1, 3, GL_FLOAT, GL_FALSE, 3 * vecSize, (void*)0);
		glVertexAttribPointer(ATTR_INSTANCE_NORMAL_MATRIX_2, 3, GL_FLOAT, GL_FALSE, 3 * vecSize, (void*)(1 * vecSize));
		glVertexAttribPointer(ATTR_INSTANCE_NORMAL_MATRIX_3, 3, GL_FLOAT, GL_FALSE, 3 * vecSize, (void*)(2 * vecSize));

		glVertexAttribDivisor(ATTR_INSTANCE_NORMAL_MATRIX_1, 1);
		glVertexAttribDivisor(ATTR_INSTANCE_NORMAL_MATRIX_2, 1);
		glVertexAttribDivisor(ATTR_INSTANCE_NORMAL_MATRIX_3, 1);

		glEnableVertexAttribArray(ATTR_INSTANCE_NORMAL_MATRIX_1);
		glEnableVertexAttribArray(ATTR_INSTANCE_NORMAL_MATRIX_2);
		glEnableVertexAttribArray(ATTR_INSTANCE_NORMAL_MATRIX_3);
	}

	if (clearBuffer) {
		modelMatrices.clear();
		normalMatrices.clear();
		dirty = false;
	}
	else {
		dirty = !modelMatrices.empty();
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Batch::update(unsigned int count)
{
	if (size == 0) return;
	//if (!dirty) return;

	if (count > size) {
		count = size;
	}

	//glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(glm::mat4), &modelMatrices[0]);
	KI_GL_CALL(glNamedBufferSubData(modelBuffer, 0, count * sizeof(glm::mat4), &modelMatrices[0]));

	//glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(glm::mat3), &normalMatrices[0]);
	KI_GL_CALL(glNamedBufferSubData(normalBuffer, 0, count * sizeof(glm::mat3), &normalMatrices[0]));

	KI_GL_UNBIND(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void Batch::bind(const RenderContext& ctx, Shader* shader)
{
	if (size == 0) return;

	//shader->drawInstanced.set(true);

	if (clearBuffer) {
		modelMatrices.clear();
		normalMatrices.clear();
	}
}

void Batch::draw(const RenderContext& ctx, Node* node, Shader* shader)
{
	if (size == 0) {
		node->bind(ctx, shader);
		node->draw(ctx);
		return;
	}

	node->bindBatch(ctx, *this);

	if (modelMatrices.size() < size) return;

	flush(ctx, node->type);
}

void Batch::flush(const RenderContext& ctx, NodeType* type)
{
	if (size == 0 || modelMatrices.empty()) return;
	
	update(modelMatrices.size());
	type->mesh->drawInstanced(ctx, modelMatrices.size());

	if (clearBuffer) {
		modelMatrices.clear();
		normalMatrices.clear();
	}
}
