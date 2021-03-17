#include "Batch.h"

#include "model/Node.h"
#include "NodeType.h"

Batch::Batch()
{
}

void Batch::add(const glm::mat4& model, const glm::mat3& normal, int objectID)
{
	modelMatrices.push_back(model);

	if (objectId) {
		int r = (objectID & 0x000000FF) >> 0;
		int g = (objectID & 0x0000FF00) >> 8;
		int b = (objectID & 0x00FF0000) >> 16;

		objectIDs.emplace_back(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
	} else {
		normalMatrices.push_back(normal);
	}
}

void Batch::reserve(int count)
{
	modelMatrices.reserve(count);
	normalMatrices.reserve(count);
	objectIDs.reserve(count);
}

int Batch::size()
{
	return modelMatrices.size();
}

void Batch::prepare(NodeType* type)
{
	if (staticBuffer) {
		batchSize = modelMatrices.size();
	}

	if (batchSize == 0) return;
	if (prepared) return;
	prepared = true;

	KI_GL_CALL(glBindVertexArray(type->mesh->buffers.VAO));

	// model
	{
		modelMatrices.reserve(batchSize);

		glCreateBuffers(1, &modelBuffer);
		glNamedBufferStorage(modelBuffer, batchSize * sizeof(glm::mat4), nullptr, GL_DYNAMIC_STORAGE_BIT);

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
		normalMatrices.reserve(batchSize);

		glCreateBuffers(1, &normalBuffer);
		glNamedBufferStorage(normalBuffer, batchSize * sizeof(glm::mat3), nullptr, GL_DYNAMIC_STORAGE_BIT);

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
		objectIDs.reserve(batchSize);

		glCreateBuffers(1, &objectIDBuffer);
		glNamedBufferStorage(objectIDBuffer, batchSize * sizeof(glm::vec4), nullptr, GL_DYNAMIC_STORAGE_BIT);

		glBindBuffer(GL_ARRAY_BUFFER, objectIDBuffer);

		glVertexAttribPointer(ATTR_INSTANCE_OBJECT_ID, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);

		glVertexAttribDivisor(ATTR_INSTANCE_OBJECT_ID, 1);

		glEnableVertexAttribArray(ATTR_INSTANCE_OBJECT_ID);
	}

	if (staticBuffer) {
		update(batchSize);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Batch::update(unsigned int count)
{
	if (batchSize == 0) return;

	if (count > batchSize) {
		KI_WARN_SB("BATCH::CUT_OFF_BUFFER: count=" << count << " batchSize=" << batchSize);
		count = batchSize;
	}

	glNamedBufferSubData(modelBuffer, 0, count * sizeof(glm::mat4), &modelMatrices[0]);

	if (objectId) {
		glNamedBufferSubData(objectIDBuffer, 0, count * sizeof(glm::vec4), &objectIDs[0]);
	}
	else {
		glNamedBufferSubData(normalBuffer, 0, count * sizeof(glm::mat3), &normalMatrices[0]);
	}

	KI_GL_UNBIND(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void Batch::bind(const RenderContext& ctx, Shader* shader)
{
	if (batchSize == 0) return;

	if (!staticBuffer) {
		modelMatrices.clear();
		normalMatrices.clear();
		objectIDs.clear();
	}
}

void Batch::draw(const RenderContext& ctx, Node* node, Shader* shader)
{
	if (batchSize == 0) {
		node->bind(ctx, shader);
		node->draw(ctx);
		return;
	}

	node->bindBatch(ctx, *this);

	if (modelMatrices.size() < batchSize) return;

	flush(ctx, node->type);
}

void Batch::flush(const RenderContext& ctx, NodeType* type)
{
	if (batchSize == 0 || modelMatrices.empty()) return;
	
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
