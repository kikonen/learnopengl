#pragma once

#include <vector>
#include <string>

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"


struct PrepareContext;
class RenderContext;

class Program;

namespace render {
    class CubeMap
    {
    public:
        CubeMap(
            std::string_view name,
            bool empty);

        ~CubeMap();

        bool valid() { return m_cubeTexture > 0; }

        void prepareRT(
            const PrepareContext& ctx);

        void bindTexture(kigl::GLState& state, int unitIndex);
        void unbindTexture(kigl::GLState& state, int unitIndex);

        operator int() const { return m_cubeTexture; }

    private:
        void createEmpty();

        void createFaces();

    public:
        const bool m_empty;
        std::string m_name;

        std::vector<std::string> m_faces;
        int m_size = 0;

        GLenum m_internalFormat = GL_RGB8;

        kigl::GLTextureHandle m_cubeTexture;
    };
}
