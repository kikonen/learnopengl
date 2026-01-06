#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "ki/size.h"
#include "kigl/kigl.h"

// TerrainTile entry
//
// NOTE KI SSBO array alignment
// - OpenGL Programming Guide, 8th Edition, page 887
// - https://stackoverflow.com/questions/23628259/how-to-properly-pad-and-align-data-in-opengl-for-std430-layout
//
// "Structure alignment is the same as the
// alignment for the biggest structure
// member, where three - component
// vectors are not rounded up to the size of
// four - component vectors.Each structure
// will start on this alignment, and its size
// will be the space needed by its
// members, according to the previous
// rules, rounded up to a multiple of the
// structure alignment."
//
// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
//
#pragma pack(push, 1)
struct TerrainTileSSBO {
    GLuint u_tileX{ 0 };
    GLuint u_tileY{ 0 };

    float u_rangeYmin{ 0.f };
    float u_rangeYmax{ 0.f };
};
#pragma pack(pop)
