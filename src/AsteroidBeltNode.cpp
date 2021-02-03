#include "AsteroidBeltNode.h"


AsteroidBeltNode::AsteroidBeltNode(Mesh* mesh)
	: Node(mesh)
{
}

void AsteroidBeltNode::setup()
{
	glm::vec3 planetPos = planet ? planet->getPos() : glm::vec3(0.f, 40.f, 0.f);

	unsigned int amount = 1000;
	srand(glfwGetTime()); // initialize random seed	

	float radius = 70.0;
	float offset = 20.5f;
	for (unsigned int i = 0; i < amount; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f;

		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;

		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z

		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;

		model = glm::translate(model, glm::vec3(x, y, z) + planetPos);

		// 2. scale: scale between 0.05 and 0.25f
		float scale = (rand() % 20) / 100.0f + 0.05;
		glm::mat4 plainModel = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = (rand() % 360);
		plainModel = glm::rotate(plainModel, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		glm::mat4 selectionModel = glm::scale(model, glm::vec3(scale * 1.02));
		selectionModel = glm::rotate(selectionModel, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. now add to list of matrices
		asteroidMatrices.push_back(plainModel);
		selectionMatrices.push_back(selectionModel);
	}
}

void AsteroidBeltNode::prepareBuffer(std::vector<glm::mat4> matrices)
{
	glBufferData(GL_ARRAY_BUFFER, matrices.size() * sizeof(glm::mat4), &matrices[0], GL_STATIC_DRAW);

	// vertex attributes
	std::size_t vec4Size = sizeof(glm::vec4);

	// NOTE mat4 as vertex attributes *REQUIRES* hacky looking approach
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);

	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));

	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));

	glEnableVertexAttribArray(9);
	glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);
	glVertexAttribDivisor(8, 1);
	glVertexAttribDivisor(9, 1);
}

void AsteroidBeltNode::prepare()
{
	Node::prepare();

	setup();

	{
		glGenBuffers(1, &asteroidBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, asteroidBuffer);
		glBindVertexArray(mesh->buffers.VAO);
		prepareBuffer(asteroidMatrices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	{
		selectedBuffers.prepare();

		mesh->prepareBuffers(selectedBuffers);

		glGenBuffers(1, &selectedBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, selectedBuffer);
		glBindVertexArray(selectedBuffers.VAO);
		prepareBuffer(selectionMatrices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}

ShaderInfo* AsteroidBeltNode::bind(const RenderContext& ctx, Shader* shader)
{
	ShaderInfo* info = Node::bind(ctx, shader);

	if (info->shader->selection) {
		glBindVertexArray(selectedBuffers.VAO);
	}

	return info;
}

void AsteroidBeltNode::draw(const RenderContext& ctx)
{
	mesh->bound->shader->drawInstanced.set(true);
	drawInstanced(ctx, asteroidMatrices.size());
}
