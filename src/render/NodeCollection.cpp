#include "NodeCollection.h"

#include "model/Node.h"

#include "mesh/MeshType.h"


namespace render {
    NodeCollection::NodeCollection() = default;

    NodeCollection::~NodeCollection()
    {
        m_solidNodes.clear();
        m_blendedNodes.clear();
        m_invisibleNodes.clear();
    }

    void NodeCollection::handleNodeAdded(Node* node)
    {
        auto* type = node->m_typeHandle.toType();

        if (type->m_flags.invisible) {
            insertNode(&m_invisibleNodes, node);
        }
        else {
            if (type->m_flags.anySolid) {
                insertNode(&m_solidNodes, node);
            }
            if (type->m_flags.anyAlpha) {
                insertNode(&m_alphaNodes, node);
            }
            if (type->m_flags.anyBlend) {
                insertNode(&m_blendedNodes, node);
            }
        }
    }

    void NodeCollection::insertNode(
        MeshTypeMap* map,
        Node* node)
    {
        auto& list = (*map)[node->m_typeHandle];
        list.reserve(100);
        list.push_back(node->toHandle());
    }
}
