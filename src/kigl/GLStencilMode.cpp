#include "GLStencilMode.h"

void GLStencilMode::apply() {
    glStencilOp(op_sfail, op_dpfail, op_dppass);
    glStencilFunc(func, func_ref, func_mask);
    glStencilMask(mask);
}

GLStencilMode GLStencilMode::fill(GLuint ref, GLuint funcMask, GLuint mask)
{
    GLStencilMode stencil;
    stencil.op_dppass = GL_REPLACE;
    stencil.func_ref = ref;
    stencil.func_mask = funcMask;
    stencil.mask = mask;
    return stencil;
}

GLStencilMode GLStencilMode::only(GLuint ref, GLuint funcMask)
{
    GLStencilMode stencil;
    stencil.func = GL_EQUAL;
    stencil.func_ref = ref;
    stencil.func_mask = funcMask;

    return stencil;
}

GLStencilMode GLStencilMode::except(GLuint ref, GLuint funcMask)
{
    GLStencilMode stencil;
    stencil.func = GL_NOTEQUAL;
    stencil.func_ref = ref;
    stencil.func_mask = funcMask;

    return stencil;
}

GLStencilMode GLStencilMode::only_non_zero(GLuint funcMask)
{
    GLStencilMode stencil;
    stencil.func = GL_NOTEQUAL;
    stencil.func_ref = 0;
    stencil.func_mask = funcMask;

    return stencil;
}
