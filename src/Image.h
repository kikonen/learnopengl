#pragma once

#include <string>


class Image final
{
public:
	Image(const std::string& path);
	~Image();

	int load(bool flip);
		
	static Image* getImage(const std::string& path);
public:
	const std::string path;

	int width = 0;
	int height = 0;
	int channels = 0;

	bool flipped = false;
	unsigned char* data = nullptr;

private:
	bool loaded = false;
	int res = 0;
};

