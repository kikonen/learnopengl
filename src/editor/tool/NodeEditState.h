#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"

#include "pool/NodeHandle.h"

namespace animation {
    struct RigSocket;
    struct Animation;
    struct Clip;
}

namespace mesh {
    class Mesh;
}

namespace editor {
    struct NodeEditState {
        pool::NodeHandle m_selectedNode;

        mesh::Mesh* m_selectedMesh{ nullptr };
        int m_selectedSocketIndex{ -1 };

        int m_selectedAnimationIndex{ -1 };
        int m_selectedClipIndex{ -1 };
    };
}
