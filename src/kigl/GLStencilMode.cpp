#include "GLStencilMode.h"

#include "GLState.h"


GLStencilMode GLStencilMode::defaults()
{
    return {};
}

GLStencilMode GLStencilMode::fill(GLuint ref, GLuint funcMask, GLuint mask)
{
    GLStencilMode stencil;
    stencil.testEnabled = true;
    stencil.op.dppass = GL_REPLACE;
    stencil.func.ref = ref;
    stencil.func.mask = funcMask;
    stencil.mask.mask = mask;

    return stencil;
}

GLStencilMode GLStencilMode::only(GLuint ref, GLuint funcMask)
{
    GLStencilMode stencil;
    stencil.testEnabled = true;
    stencil.func.func = GL_EQUAL;
    stencil.func.ref = ref;
    stencil.func.mask = funcMask;

    return stencil;
}

GLStencilMode GLStencilMode::except(GLuint ref, GLuint funcMask)
{
    GLStencilMode stencil;
    stencil.testEnabled = true;
    stencil.func.func = GL_NOTEQUAL;
    stencil.func.ref = ref;
    stencil.func.mask = funcMask;

    return stencil;
}

GLStencilMode GLStencilMode::only_non_zero(GLuint funcMask)
{
    GLStencilMode stencil;
    stencil.testEnabled = true;
    stencil.func.func = GL_NOTEQUAL;
    stencil.func.ref = 0;
    stencil.func.mask = funcMask;

    return stencil;
}
