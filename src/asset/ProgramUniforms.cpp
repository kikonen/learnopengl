#include "ProgramUniforms.h"

ProgramUniforms::ProgramUniforms(Program& program)
    : u_effect{ "u_effect", GL_FRAGMENT_SHADER, SUBROUTINE_EFFECT },
    u_nearPlane{ "u_nearPlane", UNIFORM_NEAR_PLANE },
    u_farPlane{ "u_farPlane", UNIFORM_FAR_PLANE },
    u_drawParametersIndex{ "u_drawParametersIndex", UNIFORM_DRAW_PARAMETERS_INDEX },
    u_toneHdri{ "u_toneHdri", UNIFORM_TONE_HDRI },
    u_gammaCorrect{ "u_gammaCorrect", UNIFORM_GAMMA_CORRECT },
    u_viewportTransform{ "u_viewportTransform", UNIFORM_VIEWPORT_TRANSFORM },
    u_modelMatrix{ "u_modelMatrix", UNIFORM_MODEL_MATRIX },
    u_materialIndex{ "u_materialIndex", UNIFORM_MATERIAL_INDEX },
    u_stencilMode{ "u_stencilMode", UNIFORM_STENCIL_MODE },
    u_effectBloomIteration{ "u_effectBloomIteration", UNIFORM_EFFECT_BLOOM_ITERATION }
{
    u_effect.init(&program);
}

