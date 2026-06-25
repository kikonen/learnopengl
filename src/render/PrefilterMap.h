#pragma once

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"

struct PrepareContext;

class Program;
class CubeMap;

namespace render {
    class RenderContext;

    // NOTE KI https://forums.cgsociety.org/t/gamma-and-hdri/959636
    // - hdri is *linear*
    class PrefilterMap
    {
    public:
        static constexpr int MAX_MIP_LEVELS = 5;

        PrefilterMap() = default;
        ~PrefilterMap() = default;

        bool valid() { return m_cubeTexture.valid(); }

        // skybox path: create + convolve once from m_envCubeMapID
        void prepareRT(
            const PrepareContext& ctx);

        // allocate the cube texture (with mip chain) once (probe path); size from caller
        void createRT(int size);

        // (re)convolve the given input cube into this map's mip chain; safe to call repeatedly
        void convolve(int envCubeMapID);

        // convolve a single mip level only (amortized probe baking); mip in [0, MAX_MIP_LEVELS)
        void convolveMip(int envCubeMapID, int mip);

        void bindTexture(
            kigl::GLState& state,
            int unitIndex);

        operator int() const { return m_cubeTexture; }

    private:
        void render(
            Program* program,
            int cubeTextureID,
            int baseSize);

        // render one mip level (all 6 faces) for the given roughness; self-contained FBO
        void renderMip(
            Program* program,
            int cubeTextureID,
            int baseSize,
            int mip);

    public:
        int m_size{ 0 };

        kigl::GLTextureHandle m_cubeTexture;

        int m_envCubeMapID{ 0 };
    };
}
