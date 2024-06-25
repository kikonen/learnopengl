#pragma once

#include "ki/size.h"

namespace render {
    struct DebugContext {
        int m_entityId{ 0 };
        int m_boneIndex{ 0 };

        bool m_debugBoneWeight{ false };
    };
}
