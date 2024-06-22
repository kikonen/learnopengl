#pragma once

namespace mesh {
    struct TypeFlags {
        bool anySolid: 1 {false};
        bool anyAlpha : 1 {false};
        bool anyBlend : 1 {false};

        bool preDepth : 1 {false};
        bool useBones : 1 {false};
        bool useBonesDebug : 1 {false};
        bool useAnimation : 1 {false};
        bool zUp : 1 {false};

        // NOTE KI contained node is in the context of container, managing
        // list of transforms rooted into container
        // NOTE KI container itself IS in hiearchy
        bool contained : 1 {false};

        bool mirror : 1 {false};
        bool water : 1 {false};
        bool terrain : 1 {false};
        bool cubeMap : 1 {false};
        bool effect : 1 {false};
        bool skybox : 1 {false};

        bool noShadow : 1 {false};
        bool noSelect : 1 {false};
        bool noReflect : 1 {false};
        bool noRefract : 1 {false};
        // invisible == permanently invisible
        bool invisible : 1 {false};
        bool noFrustum : 1 {false};
        bool noNormals : 1 {false};

        bool tessellation : 1 {false};

        bool physics : 1 {false};
        bool staticBounds : 1 {false};
        bool dynamicBounds : 1 {false};
    };
}
