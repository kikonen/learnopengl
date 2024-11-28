#pragma once

struct MaterialFlags {
    bool renderBack : 1 {false};
    bool lineMode : 1 {false};
    bool reverseFrontFace : 1 {false};
    bool noDepth : 1 {false};
};
