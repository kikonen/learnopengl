#pragma once

class Program;

namespace render {
    class CubeRender {
    public:
        void render(
            Program* program,
            int cubeTextureID,
            int size);
    };
}
