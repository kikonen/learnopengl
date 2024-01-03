#pragma once

class Program;

namespace kigl {
    class GLState;
}

class CubeRender {
public:
    void render(
        kigl::GLState& state,
        Program* program,
        int cubeTextureID,
        int size);
};
