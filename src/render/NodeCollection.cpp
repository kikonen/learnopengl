#include "NodeCollection.h"

#include "model/Node.h"

#include "component/CameraComponent.h"
#include "component/Light.h"

namespace {
    constexpr int INITIAL_SIZE = 1000;
}

namespace render {
    //// https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
    //struct NodeTypeComparator {
    //    bool operator()(const mesh::NodeType* a, const mesh::NodeType* b) const {
    //        return a->m_handle.m_id < b->m_handle.m_id;
    //    }
    //};

    //bool NodeTypeKey::operator<(const NodeTypeKey& o) const {
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
        m_invisibleNodes.clear();
        m_solidNodes.clear();
        m_alphaNodes.clear();
        m_blendedNodes.clear();

        m_invisibleNodes.reserve(INITIAL_SIZE);
        m_solidNodes.reserve(INITIAL_SIZE);
        m_alphaNodes.reserve(INITIAL_SIZE);
        m_blendedNodes.reserve(INITIAL_SIZE);

        m_waterNodes.clear();
        m_mirrorNodes.clear();
        m_cubeMapNodes.clear();

        m_activeCameraNode.reset();
        m_cameraNodes.clear();

        m_dirLightNodes.clear();
        m_pointLightNodes.clear();
        m_spotLightNodes.clear();
    }

    void NodeCollection::updateRT(const UpdateContext& ctx)
    {
        for (auto& handle : m_cameraNodes) {
            auto* node = handle.toNode();
            if (!node) continue;
            node->m_camera->updateRT(ctx, *node);
        }
        for (auto& handle : m_pointLightNodes) {
            auto* node = handle.toNode();
            if (!node) continue;
            node->m_light->updateRT(ctx, *node);
        }
        for (auto& handle : m_spotLightNodes) {
            auto* node = handle.toNode();
            if (!node) continue;
            node->m_light->updateRT(ctx, *node);
        }
        for (auto& handle : m_dirLightNodes) {
            auto* node = handle.toNode();
            if (!node) continue;
            node->m_light->updateRT(ctx, *node);
        }
    }

    void NodeCollection::handleNodeAdded(model::Node* node)
    {
        if (!node) return;
        auto nodeHandle = node->toHandle();

        if (node->m_typeFlags.invisible) {
            insertNode(&m_invisibleNodes, nodeHandle);
        }
        else {
            if (node->m_typeFlags.anySolid) {
                insertNode(&m_solidNodes, nodeHandle);
            }
            else if (node->m_typeFlags.anyAlpha) {
                insertNode(&m_alphaNodes, nodeHandle);
            }

            // NOTE KI node may be alpha + blend (OIT)
            if (node->m_typeFlags.anyBlend) {
                insertNode(&m_blendedNodes, nodeHandle);
            }
        }

        if (node->m_typeFlags.water) {
            m_waterNodes.push_back(nodeHandle);
        }
        if (node->m_typeFlags.mirror) {
            m_mirrorNodes.push_back(nodeHandle);
        }
        if (node->m_typeFlags.cubeMap) {
            m_cubeMapNodes.push_back(nodeHandle);
        }

        if (node->m_camera) {
            m_cameraNodes.push_back(nodeHandle);
            if (!m_activeCameraNode && node->m_camera->isDefault()) {
                setActiveCameraNode(nodeHandle);
            }
        }

        if (node->m_light) {
            Light* light = node->m_light.get();

            if (light->isDirectional()) {
                m_dirLightNodes.push_back(nodeHandle);
            }
            else if (light->isPoint()) {
                m_pointLightNodes.push_back(nodeHandle);
            }
            else if (light->isSpot()) {
                m_spotLightNodes.push_back(nodeHandle);
            }
        }
    }

    void NodeCollection::handleNodeRemoved(model::Node* node)
    {
        if (!node) return;
        auto nodeHandle = node->toHandle();

        nodeHandle.removeFrom(m_invisibleNodes);
        nodeHandle.removeFrom(m_solidNodes);
        nodeHandle.removeFrom(m_alphaNodes);
        nodeHandle.removeFrom(m_blendedNodes);

        nodeHandle.removeFrom(m_waterNodes);
        nodeHandle.removeFrom(m_mirrorNodes);
        nodeHandle.removeFrom(m_cubeMapNodes);

        nodeHandle.removeFrom(m_cameraNodes);

        nodeHandle.removeFrom(m_dirLightNodes);
        nodeHandle.removeFrom(m_pointLightNodes);
        nodeHandle.removeFrom(m_spotLightNodes);
    }

    void NodeCollection::insertNode(
        NodeVector* handles,
        pool::NodeHandle nodeHandle)
    {
        nodeHandle.addTo(*handles);
    }

    void NodeCollection::setActiveCameraNode(pool::NodeHandle nodeHandle)
    {
        if (!nodeHandle) {
            nodeHandle = findDefaultCameraNode();
        }

        auto* node = nodeHandle.toNode();
        if (!node) return;
        if (!node->m_camera) return;

        m_activeCameraNode = nodeHandle;
    }

    pool::NodeHandle NodeCollection::getNextCameraNode(
        pool::NodeHandle srcNode,
        int offset) const noexcept
    {
        int index = 0;
        int size = static_cast<int>(m_cameraNodes.size());
        for (int i = 0; i < size; i++) {
            if (m_cameraNodes[i] == srcNode) {
                index = std::max(0, (i + offset) % size);
                break;
            }
        }
        return m_cameraNodes[index];
    }

    pool::NodeHandle NodeCollection::findDefaultCameraNode() const
    {
        const auto& it = std::find_if(
            m_cameraNodes.begin(),
            m_cameraNodes.end(),
            [](pool::NodeHandle handle) {
                auto* node = handle.toNode();
                return node && node->m_camera->isDefault();
            });
        return it != m_cameraNodes.end() ? *it : pool::NodeHandle::NULL_HANDLE;
    }
}
