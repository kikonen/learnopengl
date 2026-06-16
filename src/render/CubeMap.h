#pragma once

#include <vector>
#include <string>

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"

class Program;
struct PrepareContext;

namespace render {
    class RenderContext;

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

        // mip levels for an empty cube (1 = no mips). When > 1 the min filter is set to
        // mipmap-linear; the caller fills mips (e.g. glGenerateTextureMipmap after rendering).
        int m_levels = 1;

        GLenum m_internalFormat = GL_RGB8;

        kigl::GLTextureHandle m_cubeTexture;
    };
}
