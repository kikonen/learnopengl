#pragma once


class FrameBuffer final
{
public:
	FrameBuffer(int width, int height);
	~FrameBuffer();

	void prepare();
	void bind();
	void unbind();

	void bindTexture(int unitID);

public:
	const int width;
	const int height;

	unsigned int FBO;
	unsigned int textureID;

private:
	bool prepared = false;
};

