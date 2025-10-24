#pragma once

namespace mesh {
    struct MeshFlags {
        bool billboard : 1 {false};
        bool tessellation : 1 {false};

        bool preDepth : 1 {false};

        bool noVolume : 1{false};

        bool useJoints : 1 {false};
        bool useJointsDebug : 1 {false};
        bool useAnimation : 1 {false};
        bool jointVisualization : 1 {false};

        bool useSockets : 1 {false};

        bool clip : 1 {false};
    };
}
