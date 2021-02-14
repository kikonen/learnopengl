#pragma once

#include "NodeController.h"

class CameraController : public NodeController
{
public:
	CameraController(const Assets& assets);
	~CameraController();
};
