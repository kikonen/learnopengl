#pragma once

namespace mesh {
    struct MeshFlags {
        bool hidden : 1 {false};

        bool billboard : 1 {false};
        bool tessellation : 1 {false};

        bool preDepth : 1 {false};

        bool zUp : 1 {false};

        bool noVolume : 1{false};

        bool useBones : 1 {false};
        bool useBonesDebug : 1 {false};
        bool useAnimation : 1 {false};
    };
}
