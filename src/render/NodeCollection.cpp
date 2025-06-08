#include "NodeCollection.h"

#include "model/Node.h"

namespace render {
    //// https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
    //struct MeshTypeComparator {
    //    bool operator()(const mesh::MeshType* a, const mesh::MeshType* b) const {
    //        return a->m_handle.m_id < b->m_handle.m_id;
    //    }
    //};

    //bool MeshTypeKey::operator<(const MeshTypeKey& o) const {
    //    const auto& a = m_typeHandle.toType();
    //    const auto& b = o.m_typeHandle.toType();

    //    return a->m_handle.m_id < b->m_handle.m_id;
    //}

    NodeCollection::NodeCollection() = default;

    NodeCollection::~NodeCollection()
    {
        clear();
    }

    void NodeCollection::clear()
    {
        m_solidNodes.clear();
        m_alphaNodes.clear();
        m_blendedNodes.clear();
        //m_invisibleNodes.clear();

        m_waterNodes.clear();
        m_mirrorNodes.clear();
        m_cubeMapNodes.clear();
    }

    void NodeCollection::handleNodeAdded(Node* node)
    {
        if (node->m_typeFlags.invisible) {
            insertNode(&m_invisibleNodes, node);
        }
        else {
            if (node->m_typeFlags.anySolid) {
                insertNode(&m_solidNodes, node);
            }
            if (node->m_typeFlags.anyAlpha) {
                insertNode(&m_alphaNodes, node);
            }
            if (node->m_typeFlags.anyBlend) {
                insertNode(&m_blendedNodes, node);
            }
        }

        if (node->m_typeFlags.water) {
            m_waterNodes.push_back(node->toHandle());
        }
        if (node->m_typeFlags.mirror) {
            m_mirrorNodes.push_back(node->toHandle());
        }
        if (node->m_typeFlags.cubeMap) {
            m_cubeMapNodes.push_back(node->toHandle());
        }
    }

    void NodeCollection::insertNode(
        NodeVector* nodes,
        Node* node)
    {
        nodes->reserve(100);
        nodes->push_back(node->toHandle());
    }
}
