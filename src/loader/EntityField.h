#pragma once

namespace loader {
    struct EntityField {
        bool enabled{ false };
        bool active{ false };
        bool type{ false };

        bool baseId{ false };
        bool parentBaseId{ false };

        bool name{ false };

        bool type{ false };

        bool name{ false };
        bool desc{ false };

        bool prefab{ false };

        bool priority{ false };

        bool lods{ false };

        bool programName{ false };
        bool geometryType{ false };

        bool shadowProgramName{ false };
        bool preDepthProgramName{ false };

        bool programDefinitions{ false };
        bool renderFlags{ false };

        bool baseRotation{ false };

        bool position{ false };
        bool rotation{ false };

        bool front{ false };
        bool scale{ false };
        bool baseScale{ false };

        bool selected{ false };
        bool cloneMesh{ false };

        bool tiling{ false };
        bool tile{ false };

        bool clonePositionOFfset{ false };

        bool customMaterial{ false };
        bool physics{ false };

        bool repeat{ false };

        bool script{ false };

        bool controllers{ false };
        bool camera{ false };
        bool light{ false };
        bool audio{ false };
        bool text{ false };
        bool animations{ false };
        bool generator{ false };
        bool particle{ false };
    };
}
