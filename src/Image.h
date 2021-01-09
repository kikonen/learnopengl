#pragma once

#include <string>


class Image
{
public:
	Image(const std::string& path);
	~Image();

	int load();
		
	static Image* getImage(const std::string& path);
public:
	const std::string path;

	int width = 0;
	int height = 0;
	int channels = 0;

	unsigned char* data = nullptr;

private:
	bool loaded = false;
	int res = 0;
};

