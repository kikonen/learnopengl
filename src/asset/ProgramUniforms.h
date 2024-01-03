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

    uniform::UInt u_shadowIndex;
    uniform::Subroutine u_effect;

    uniform::Float u_nearPlane;
    uniform::Float u_farPlane;

    uniform::UInt u_drawParametersIndex;

    uniform::UInt u_effectBloomIteration;

    uniform::Bool u_toneHdri;
    uniform::Bool u_gammaCorrect;
    uniform::Mat4 u_viewportTransform;

    uniform::Mat4 u_modelMatrix;
    uniform::UInt u_materialIndex;

    uniform::Int u_stencilMode;
};

