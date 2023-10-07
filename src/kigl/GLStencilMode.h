#pragma once

#include "kigl/kigl.h"

// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glStencilOp.xhtml
// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glStencilFunc.xhtml
// https://registry.khronos.org/OpenGL-Refpages/es2.0/xhtml/glStencilMask.xml
struct GLStencilMode {
    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    GLenum op_sfail{ GL_KEEP };
    GLenum op_dpfail{ GL_KEEP };
    GLenum op_dppass{ GL_KEEP };

    // glStencilFunc(GL_ALWAYS, STENCIL_HIGHLIGHT, 0xff);
    GLenum func{ GL_ALWAYS };
    GLint func_ref{ 0 };
    GLuint func_mask{ 0xff };

    // glStencilMask(0xff);
    GLuint mask{ 0xff };

    bool operator==(const GLStencilMode& o) {
        return op_sfail == o.op_sfail &&
            op_dpfail == o.op_dpfail &&
            op_dppass == o.op_dppass &&
            func == o.func &&
            func_ref == o.func_ref &&
            func_mask == o.func_mask &&
            mask == o.mask;
    }

    bool operator!=(const GLStencilMode& o) {
        return !operator==(o);
    }

    void apply();

    static GLStencilMode fill(GLuint ref, GLuint funcMask = 0xff, GLuint mask = 0xff);
    static GLStencilMode only(GLuint ref, GLuint funcMask = 0xff);
    static GLStencilMode except(GLuint ref, GLuint funcMask = 0xff);
    static GLStencilMode only_non_zero(GLuint funcMask = 0xff);
};
