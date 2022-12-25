#pragma once

#pragma pack(push, 1)
struct MaterialEntry {
    // NOTE KI uint DOES NOT work well in vertex attrs; data gets corrupted
    // => use float
    float materialIndex;
};
#pragma pack(pop)
