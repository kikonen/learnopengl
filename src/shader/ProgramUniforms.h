#pragma once

#include "Uniform.h"

//namespace uniform {
//    class Uniform;
//    class Subroutine;
//    class Float;
//    class Int;
//    class UInt;
//    class Bool;
//    class Mat4;
//}

class Program;

struct ProgramUniforms {
    ProgramUniforms(Program& program);

    uniform::Subroutine u_effect;

    uniform::Float u_shadowNearPlane;
    uniform::Float u_shadowFarPlane;

    uniform::UInt u_drawParametersIndex;

    uniform::Bool u_effectBloomHorizontal;

    uniform::Bool u_hdrToneEnabled;
    uniform::Bool u_gammaCorrectEnabled;

    uniform::Mat4 u_viewportTransform;

    uniform::Mat4 u_modelMatrix;
    uniform::UInt u_materialIndex;

    uniform::Int u_stencilMode;

    uniform::Float u_blendFactor;

    uniform::Vec2 u_viewport;
};

