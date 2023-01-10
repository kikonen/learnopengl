#pragma once

#pragma pack(push, 1)
struct MaterialIndex {
    // NOTE KI uint DOES NOT work well in vertex attrs; data gets corrupted
    // => use float
    float m_materialIndex;
};
#pragma pack(pop)