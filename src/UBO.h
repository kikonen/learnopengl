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
const unsigned int UBO_FLOAT_SIZE = sizeof(float);
const unsigned int UBO_BOOL_SIZE = 4;

const unsigned int MAX_LIGHTS = 16;

enum struct DirLightUBO {
	dir = 0,
	ambient = UBO_VEC_SIZE * 1,
	diffuse = UBO_VEC_SIZE * 2,
	specular = UBO_VEC_SIZE * 3,
	use = UBO_VEC_SIZE * 4,
	sz = UBO_VEC_SIZE * 4 + UBO_BOOL_SIZE,
};

// NOTE KI PAD to vec4
const int SZ_DIR_LIGHT_UBO = UBO_VEC_SIZE * ((int)(DirLightUBO::sz) / UBO_VEC_SIZE) + 
	((int)(DirLightUBO::sz) % UBO_VEC_SIZE > 1 ? UBO_VEC_SIZE : 0);

enum struct PointLightUBO {
	pos = 0,

	ambient = UBO_VEC_SIZE * 1,
	diffuse = UBO_VEC_SIZE * 2,
	specular = UBO_VEC_SIZE * 3,

	constant = UBO_VEC_SIZE * 3 + UBO_FLOAT_SIZE * 0,
	linear = UBO_VEC_SIZE * 3 + UBO_FLOAT_SIZE * 1,
	quadratic = UBO_VEC_SIZE * 3 + UBO_FLOAT_SIZE * 2,

	use = UBO_VEC_SIZE * 3 + UBO_FLOAT_SIZE * 3,

	// NOTE KI PAD to vec4
	sz = UBO_VEC_SIZE * 3 + UBO_FLOAT_SIZE * 3 + UBO_BOOL_SIZE,
};

// NOTE KI PAD to vec4
const int SZ_POINT_LIGHT_UBO = UBO_VEC_SIZE * ((int)(PointLightUBO::sz) / UBO_VEC_SIZE) + 
	((int)(PointLightUBO::sz) % UBO_VEC_SIZE > 0 ? UBO_VEC_SIZE : 0);

enum struct SpotLightUBO {
	pos = 0,
	dir = UBO_VEC_SIZE * 1,

	ambient = UBO_VEC_SIZE * 2,
	diffuse = UBO_VEC_SIZE * 3,
	specular = UBO_VEC_SIZE * 4,

	constant = UBO_VEC_SIZE * 4 + UBO_FLOAT_SIZE * 0,
	linear = UBO_VEC_SIZE * 4 + UBO_FLOAT_SIZE * 1,
	quadratic = UBO_VEC_SIZE * 4 + UBO_FLOAT_SIZE * 2,

	cutoff = UBO_VEC_SIZE * 4 + UBO_FLOAT_SIZE * 3,
	outerCutoff = UBO_VEC_SIZE * 4 + UBO_FLOAT_SIZE * 4,

	use = UBO_VEC_SIZE * 4 + UBO_FLOAT_SIZE * 5,

	sz = UBO_VEC_SIZE * 4 + UBO_FLOAT_SIZE * 5 + UBO_BOOL_SIZE,
};

// NOTE KI PAD to vec4
const int SZ_SPOT_LIGHT_UBO = UBO_VEC_SIZE * ((int)(SpotLightUBO::sz) / UBO_VEC_SIZE) +
((int)(SpotLightUBO::sz) % UBO_VEC_SIZE > 1 ? UBO_VEC_SIZE : 0);
