#pragma once

#include "BaseLoader.h"

namespace loader {
    class VolumeLoader : public BaseLoader
    {
    public:
        VolumeLoader(
            Context ctx);

        void attachVolume(
            const uuids::uuid& rootId);

    };
}
