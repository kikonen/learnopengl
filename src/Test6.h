#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"
#include "Node.h"
#include "Shader.h"
#include "Light.h"

class Test6 : public Engine {
public:
	Test6();

	int onSetup() override;
	int onRender(float dt) override;

	void processInput(float dt) override;
private:
	float elapsed = 0;

	std::vector<Node*> nodes;
	Node* active;

	Node* light;
};
