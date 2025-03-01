#include "NodeCollection.h"

#include "model/Node.h"

#include "mesh/MeshType.h"


namespace render {
    //// https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
    //struct MeshTypeComparator {
    //    bool operator()(const mesh::MeshType* a, const mesh::MeshType* b) const {
    //        return a->m_handle.m_id < b->m_handle.m_id;
    //    }
    //};

    bool MeshTypeKey::operator<(const MeshTypeKey& o) const {
        const auto& a = m_typeHandle.toType();
        const auto& b = o.m_typeHandle.toType();

        return a->m_handle.m_id < b->m_handle.m_id;
    }

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
        m_invisibleNodes.clear();

        m_waterNodes.clear();
        m_mirrorNodes.clear();
        m_cubeMapNodes.clear();
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

        if (type->m_flags.water) {
            m_waterNodes.push_back(node->toHandle());
        }
        if (type->m_flags.mirror) {
            m_mirrorNodes.push_back(node->toHandle());
        }
        if (type->m_flags.cubeMap) {
            m_cubeMapNodes.push_back(node->toHandle());
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
