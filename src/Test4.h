#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"
#include "ModelMesh.h"
#include "Shader.h"

class Test4 : public Engine {
public:
	Test4();

	int onSetup() override;
	int onRender(float dt) override;

private:
	float elapsed = 0;

	ModelMesh* mesh = NULL;
};
