#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"
#include "SimpleMesh.h"
#include "Shader.h"

class Test3 : public Engine {
public:
	Test3();

	int onSetup() override;
	int onRender(float dt) override;

private:
	SimpleMesh* createElementMesh1();
	SimpleMesh* createElementMesh2();

	float elapsed = 0;

	SimpleMesh* mesh1 = NULL;
	SimpleMesh* mesh2 = NULL;
};
