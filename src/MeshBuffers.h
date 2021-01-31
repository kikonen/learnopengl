#pragma once

class MeshBuffers
{
public:
	MeshBuffers();
	~MeshBuffers();

	void prepare();
public:
	unsigned int VBO = 0;
	unsigned int VAO = 0;
	unsigned int EBO = 0;

private:
	bool prepared = false;
};

