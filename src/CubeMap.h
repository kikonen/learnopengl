#pragma once

#include <vector>
#include <string>

#include "TextureBuffer.h"


class CubeMap
{
public:
	CubeMap();

	unsigned int createFromFrameBuffers(std::vector<TextureBuffer*> faces);
	unsigned int createFromImages(std::vector<std::string> faces);

public:
	unsigned int textureID;
};

