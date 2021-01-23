#include "AsteroidBeltNode.h"


AsteroidBeltNode::AsteroidBeltNode(ModelMesh* mesh) : Node(mesh)
{
	setup();
}

void AsteroidBeltNode::setup()
{
	glm::vec3 planetPos = glm::vec3(10, 100, 60);

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
		asteroidMatrixes.push_back(plainModel);
		selectionMatrixes.push_back(selectionModel);
	}
}

ShaderInfo* AsteroidBeltNode::prepare(Shader* shader)
{
	ShaderInfo* info = Node::prepare(shader);

	if (info->preparedNode) {
		return info;
	}
	bool selection = info->shader->shaderName == "stencil";
	info->preparedNode = true;

	glGenBuffers(1, &asteroidBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, asteroidBuffer);
	if (selection) {
		glBufferData(GL_ARRAY_BUFFER, selectionMatrixes.size() * sizeof(glm::mat4), &selectionMatrixes[0], GL_STATIC_DRAW);
	}
	else {
		glBufferData(GL_ARRAY_BUFFER, asteroidMatrixes.size() * sizeof(glm::mat4), &asteroidMatrixes[0], GL_STATIC_DRAW);
	}

	glBindVertexArray(info->VAO);

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

	glBindVertexArray(0);

	return info;
}

int AsteroidBeltNode::bind(const RenderContext& ctx, Shader* shader)
{
	return Node::bind(ctx, shader);
}

void AsteroidBeltNode::draw(const RenderContext& ctx)
{
	mesh->bound->shader->setBool("drawInstanced", true);
	drawInstanced(ctx, asteroidMatrixes.size());
}
