#include "Batch.h"

#include "model/Node.h"
#include "NodeType.h"

Batch::Batch()
{
}

void Batch::add(const glm::mat4& model, const glm::mat3& normal, int objectID)
{
	modelMatrices.push_back(model);
	normalMatrices.push_back(normal);

	int r = (objectID & 0xff) >> 0;
	int g = (objectID & 0xff00) >> 8;
	int b = (objectID & 0xff0000) >> 16;

	objectIDs.emplace_back(r / 255.0f, g / 255.0f, b / 255.0f);
}

void Batch::prepare(NodeType* type)
{
	if (staticBuffer) {
		size = modelMatrices.size();
	}

	if (size == 0) return;
	if (prepared) return;
	prepared = true;

	KI_GL_CALL(glBindVertexArray(type->mesh->buffers.VAO));

	// model
	{
		modelMatrices.reserve(size);

		glCreateBuffers(1, &modelBuffer);
		glNamedBufferStorage(modelBuffer, size * sizeof(glm::mat4), nullptr, GL_DYNAMIC_STORAGE_BIT);

		// NOTE mat4 as vertex attributes *REQUIRES* hacky looking approach
		size_t vecSize = sizeof(glm::vec4);

		glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);

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
		normalMatrices.reserve(size);

		glCreateBuffers(1, &normalBuffer);
		glNamedBufferStorage(normalBuffer, size * sizeof(glm::mat3), nullptr, GL_DYNAMIC_STORAGE_BIT);

		// NOTE mat3 as vertex attributes *REQUIRES* hacky looking approach
		size_t vecSize = sizeof(glm::vec3);

		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);

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

	// objectIDs
	{
		objectIDs.reserve(size);

		glCreateBuffers(1, &objectIDBuffer);
		glNamedBufferStorage(objectIDBuffer, size * sizeof(glm::vec3), nullptr, GL_DYNAMIC_STORAGE_BIT);

		glBindBuffer(GL_ARRAY_BUFFER, objectIDBuffer);

		glVertexAttribPointer(ATTR_INSTANCE_OBJECT_ID, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

		glVertexAttribDivisor(ATTR_INSTANCE_OBJECT_ID, 1);

		glEnableVertexAttribArray(ATTR_INSTANCE_OBJECT_ID);
	}

	if (staticBuffer) {
		update(size);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Batch::update(unsigned int count)
{
	if (size == 0) return;

	if (count > size) {
		KI_WARN_SB("BATCH::CUT_OFF_BUFFER: count=" << count << " size=" << size);
		count = size;
	}

	glNamedBufferSubData(modelBuffer, 0, count * sizeof(glm::mat4), &modelMatrices[0]);
	glNamedBufferSubData(normalBuffer, 0, count * sizeof(glm::mat3), &normalMatrices[0]);
	glNamedBufferSubData(objectIDBuffer, 0, count * sizeof(int), &objectIDs[0]);

	KI_GL_UNBIND(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void Batch::bind(const RenderContext& ctx, Shader* shader)
{
	if (size == 0) return;

	if (!staticBuffer) {
		modelMatrices.clear();
		normalMatrices.clear();
		objectIDs.clear();
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
	
	if (!staticBuffer) {
		update(modelMatrices.size());
	}

	type->mesh->drawInstanced(ctx, modelMatrices.size());

	if (!staticBuffer) {
		modelMatrices.clear();
		normalMatrices.clear();
		objectIDs.clear();
	}
}
