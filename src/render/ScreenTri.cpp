#include "ScreenTri.h"

#include "kigl/kigl.h"

namespace {
    static render::ScreenTri g_instance;
}

namespace render {
    ScreenTri& ScreenTri::get() noexcept
    {
        return g_instance;
    }

    void ScreenTri::draw()
    {
        // NOTE KI single triangle filling whole viewport
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}
