#pragma once

#include "kigl/kigl.h"

namespace kigl {
    struct GLStencilOp {
        // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        GLenum sfail{ GL_KEEP };
        GLenum dpfail{ GL_KEEP };
        GLenum dppass{ GL_KEEP };

        bool operator==(const GLStencilOp& o) {
            return sfail == o.sfail &&
                dpfail == o.dpfail &&
                dppass == o.dppass;
        }

        bool operator!=(const GLStencilOp& o) {
            return !operator==(o);
        }
    };

    struct GLStencilFunc {
        // glStencilFunc(GL_ALWAYS, STENCIL_HIGHLIGHT, 0xff);
        GLenum func{ GL_ALWAYS };
        GLint ref{ 0 };
        GLuint mask{ 0xff };

        bool operator==(const GLStencilFunc& o) {
            return func == o.func &&
                ref == o.ref &&
                mask == o.mask;
        }

        bool operator!=(const GLStencilFunc& o) {
            return !operator==(o);
        }
    };

    struct GLStencilMask {
        // glStencilMask(0xff);
        GLuint mask{ 0xff };

        bool operator==(const GLStencilMask& o) {
            return mask == o.mask;
        }

        bool operator!=(const GLStencilMask& o) {
            return !operator==(o);
        }
    };

    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glStencilOp.xhtml
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glStencilFunc.xhtml
    // https://registry.khronos.org/OpenGL-Refpages/es2.0/xhtml/glStencilMask.xml
    struct GLStencilMode {
        bool testEnabled{ false };

        GLStencilOp op;
        GLStencilFunc func;
        GLStencilMask mask;

        bool operator==(const GLStencilMode& o) {
            return testEnabled == o.testEnabled &&
                op == o.op &&
                func == o.func &&
                mask == o.mask;
        }

        bool operator!=(const GLStencilMode& o) {
            return !operator==(o);
        }

        static GLStencilMode defaults();
        static GLStencilMode fill(GLuint ref, GLuint funcMask = 0xff, GLuint mask = 0xff);
        static GLStencilMode only(GLuint ref, GLuint funcMask = 0xff);
        static GLStencilMode except(GLuint ref, GLuint funcMask = 0xff);
        static GLStencilMode only_non_zero(GLuint funcMask = 0xff);
    };
}
