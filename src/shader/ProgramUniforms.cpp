#include "ProgramUniforms.h"

ProgramUniforms::ProgramUniforms(Program& program)
    : u_effect{ "u_effect", GL_FRAGMENT_SHADER, SUBROUTINE_EFFECT },
    u_shadowNearPlane{ "u_shadowNearPlane", UNIFORM_SHADOW_NEAR_PLANE },
    u_shadowFarPlane{ "u_shadowFarPlane", UNIFORM_SHADOW_FAR_PLANE },
    u_drawParametersIndex{ "u_drawParametersIndex", UNIFORM_DRAW_PARAMETERS_INDEX },
    u_toneHdri{ "u_toneHdri", UNIFORM_TONE_HDRI },
    u_gammaCorrect{ "u_gammaCorrect", UNIFORM_GAMMA_CORRECT },
    u_viewportTransform{ "u_viewportTransform", UNIFORM_VIEWPORT_TRANSFORM },
    u_modelMatrix{ "u_modelMatrix", UNIFORM_MODEL_MATRIX },
    u_materialIndex{ "u_materialIndex", UNIFORM_MATERIAL_INDEX },
    u_stencilMode{ "u_stencilMode", UNIFORM_STENCIL_MODE },
    u_blendFactor{ "u_blendFactor", UNIFORM_BLEND_FACTOR },
    u_viewport{ "u_viewport", UNIFORM_VIEWPORT},
    u_effectBloomHorizontal{ "u_effectBloomHorizontal", UNIFORM_EFFECT_BLOOM_HORIZONTAL}
{
    u_effect.init(&program);

    u_shadowNearPlane.init(&program);
    u_shadowFarPlane.init(&program);
    u_drawParametersIndex.init(&program);
    u_toneHdri.init(&program);
    u_gammaCorrect.init(&program);
    u_viewportTransform.init(&program);
    u_modelMatrix.init(&program);
    u_materialIndex.init(&program);
    u_stencilMode.init(&program);
    u_blendFactor.init(&program);
    u_viewport.init(&program);
    u_effectBloomHorizontal.init(&program);
}

