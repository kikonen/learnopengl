#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"
#include "ModelMesh.h"
#include "Shader.h"
#include "Light.h"

class Test5 : public Engine {
public:
	Test5();

	int onSetup() override;
	int onRender(float dt) override;

private:
	float elapsed = 0;

	ModelMesh* mesh = NULL;

	Light light;
};
