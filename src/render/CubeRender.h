#pragma once

class Program;
class GLState;

class CubeRender {
public:
    void render(
        GLState& state,
        Program* program,
        int cubeTextureID,
        int size);
};
