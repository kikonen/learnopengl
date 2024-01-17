#pragma once

#include "BaseLoader.h"

namespace loader {
    class CubeMapLoader : public BaseLoader
    {
    public:
        CubeMapLoader(
            Context ctx);

        void attachCubeMap(
            const ki::node_id rootId);
    };
}
