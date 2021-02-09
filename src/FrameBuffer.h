#pragma once


class FrameBuffer final
{
public:
	FrameBuffer(int w, int h);
	~FrameBuffer();

	void prepare();
	void bind();
	void unbind();

	void bindTexture(int unitID);

public:
	const int w;
	const int h;

	unsigned int FBO;
	unsigned int textureID;

private:
	bool prepared = false;
};

