#pragma once

#include "ki/GL.h"

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

    void apply() {
        glStencilOp(op_sfail, op_dpfail, op_dppass);
        glStencilFunc(func, func_ref, func_mask);
        glStencilMask(mask);
    }

    static GLStencilMode fill(GLuint ref)
    {
        GLStencilMode stencil;
        stencil.op_dppass = GL_REPLACE;
        stencil.func_ref = ref;
        return stencil;
    }

    static GLStencilMode only(GLuint ref)
    {
        GLStencilMode stencil;
        stencil.op_dppass = GL_REPLACE;
        stencil.func = GL_EQUAL;
        stencil.func_ref = ref;
        stencil.mask = 0x00;

        return stencil;
    }

    static GLStencilMode except(GLuint ref)
    {
        GLStencilMode stencil;
        stencil.op_dppass = GL_REPLACE;
        stencil.func = GL_NOTEQUAL;
        stencil.func_ref = ref;
        stencil.mask = 0x00;

        return stencil;
    }

    static GLStencilMode non_zero()
    {
        GLStencilMode stencil;
        stencil.op_dppass = GL_REPLACE;
        stencil.func = GL_NOTEQUAL;
        stencil.func_ref = 0;
        stencil.mask = 0x00;

        return stencil;
    }
};
