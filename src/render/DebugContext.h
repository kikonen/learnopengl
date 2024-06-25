#pragma once

#include "ki/size.h"

namespace render {
    struct DebugContext {
        int m_entityId{ -1 };
        int m_boneIndex{ -1 };

        bool m_debugBoneWeight{ false };
    };
}
