#pragma once

#include "editor/tool/ToolState.h"

#include "pool/TypeHandle.h"

namespace animation {
    struct RigSocket;
    struct Animation;
    struct Clip;
}

namespace mesh {
    class Mesh;
}

namespace editor {
    struct NodeTypeToolState : public ToolState {
        pool::TypeHandle m_selectedType;

        mesh::Mesh* m_selectedMesh{ nullptr };
        int m_selectedSocketIndex{ -1 };

        int m_selectedAnimationIndex{ -1 };
        int m_selectedClipIndex{ -1 };
    };
}
