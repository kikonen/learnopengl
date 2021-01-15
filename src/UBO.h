#pragma once

struct UBO {
	unsigned int matrices;
	unsigned int data;
	unsigned int lights;
};

const unsigned int UBO_MATRICES = 0;
const unsigned int UBO_DATA = 1;
const unsigned int UBO_LIGHTS = 2;

const unsigned int UBO_MAT_SIZE = sizeof(glm::mat4);
const unsigned int UBO_VEC_SIZE = sizeof(glm::vec4);
const unsigned int UBO_INT_SIZE = sizeof(unsigned int);
