#pragma once

class Program;
class GLState;
class GLTextureHandle;

class CubeRender {
public:
    void render(
        GLState& state,
        Program* program,
        GLTextureHandle cubeTexture);
};
