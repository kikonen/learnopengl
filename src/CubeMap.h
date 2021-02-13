#pragma once

#include <vector>
#include <string>

#include "FrameBuffer.h"


class CubeMap
{
public:
	CubeMap();

	unsigned int createFromFrameBuffers(std::vector<FrameBuffer*> faces);
	unsigned int createFromImages(std::vector<std::string> faces);

public:
	unsigned int textureID;
};

