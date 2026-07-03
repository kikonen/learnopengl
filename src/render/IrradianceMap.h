#pragma once

#include <vector>
#include <string>

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"

#include "CubeRender.h"

class CubeMap;

struct PrepareContext;

class Program;

namespace render {
    class RenderContext;

    // NOTE KI https://forums.cgsociety.org/t/gamma-and-hdri/959636
    // - hdri is *linear*
    class IrradianceMap
    {
    public:
        IrradianceMap() = default;
        ~IrradianceMap() = default;

        bool valid() { return m_cubeTexture.valid(); }

        // skybox path: create + convolve once from m_envCubeMapID
        void prepareRT(
            const PrepareContext& ctx);

        // allocate the cube texture once (probe path); size from caller
        void createRT(int size);

        // (re)convolve the given input cube into this map; safe to call repeatedly
        void convolve(int envCubeMapID);

        void bindTexture(
            kigl::GLState& state,
            int unitIndex);

        operator int() const { return m_cubeTexture; }

    public:
        int m_size{ 0 };

        kigl::GLTextureHandle m_cubeTexture;

        int m_envCubeMapID{ 0 };

        // DEBUG KI logs source-cube / unit-70 GL state at each convolve
        bool m_debug{ false };

    private:
        // NOTE KI persistent so its capture FBO is created once and reused across
        // repeated convolves (probe re-bakes), instead of per-call create/destroy
        CubeRender m_cubeRender;
    };
}
