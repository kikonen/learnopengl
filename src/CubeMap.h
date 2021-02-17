#pragma once

#include <vector>
#include <string>


class CubeMap
{
public:
	static unsigned int createEmpty(int size);
	static unsigned int createFromImages(std::vector<std::string> faces);
};
