#include "Rig.h"

#include <tuple>

#include <assimp/scene.h>
#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/Log.h"
#include "util/util.h"
#include "util/util_join.h"
#include "util/assimp_util.h"

#include "Animation.h"
#include "RigNode.h"
#include "RigSocket.h"
#include "Joint.h"
#include "VertexJoint.h"
#include "JointContainer.h"


namespace animation
{
    void Rig::prepare()
    {
        validate();
    }

    void Rig::prepareSockets()
    {
        // TODO KI MUST pass socketnames to loader to shat they won't
        // be discarded from node tree (in case they are leaf nodes)

        // NOTE KI mesh required for calculating transforms for attached meshes
        for (auto& rigNode : m_nodes) {
            if (rigNode.m_hasJoint) {
                rigNode.m_requiredForSocket = true;
                for (auto jointIndex = rigNode.m_parentIndex; jointIndex >= 0;) {
                    auto& parent = m_nodes[jointIndex];
                    parent.m_requiredForJoint = true;
                    jointIndex = parent.m_parentIndex;
                }
            }

            if (rigNode.m_hasSocket) {
                rigNode.m_requiredForSocket = true;
                for (auto jointIndex = rigNode.m_parentIndex; jointIndex >= 0;) {
                    auto& parent = m_nodes[jointIndex];
                    parent.m_requiredForSocket = true;
                    jointIndex = parent.m_parentIndex;
                }
            }
        }

        // NOTE KI mesh required for calculating transforms for attached meshes
        for (auto& rigNode : m_nodes) {
            const auto& it = std::find_if(
                m_sockets.begin(),
                m_sockets.end(),
                [&rigNode](const auto& socket) { return socket.m_nodeIndex == rigNode.m_index; });
            if (it == m_sockets.end()) continue;

            rigNode.m_requiredForSocket = true;

            for (auto jointIndex = rigNode.m_parentIndex; jointIndex >= 0;) {
                auto& parent = m_nodes[jointIndex];
                // NOTE KI m_sockets is not sorted
                //if (parent.m_socketRequired) break;
                parent.m_requiredForSocket = true;
                jointIndex = parent.m_parentIndex;
            }
        }
    }

    void Rig::validate() const
    {
        std::vector<const animation::Joint*> unboundJoints;

        // NOTE KI check that all joints are related to some node
        // - every joint has node
        // - not every node has joint
        for (const auto& joint : m_jointContainer.m_joints) {
            if (joint.m_nodeIndex >= 0) continue;

            unboundJoints.push_back(&joint);
        }

        if (unboundJoints.empty()) {
            auto sb = util::join(
                unboundJoints, ", ",
                [](const auto* joint) {
                return joint->m_nodeName;
            });

            throw std::runtime_error(fmt::format(
                "missing_joint_nodes: {}",
                sb));
        }
    }

    const animation::RigNode* Rig::getNode(int16_t nodeIndex) const noexcept
    {
        return nodeIndex >= 0 ? &m_nodes[nodeIndex] : nullptr;
    }

    const animation::RigNode* Rig::findNode(const std::string& name) const noexcept
    {
        const auto& it = std::find_if(
            m_nodes.begin(),
            m_nodes.end(),
            [&name](const RigNode& j) {
                return j.m_name == name;
            });
        return it != m_nodes.end() ? &m_nodes[it->m_index] : nullptr;
    }

    animation::RigNode* Rig::findNode(const std::string& name) noexcept
    {
        const auto& it = std::find_if(
            m_nodes.begin(),
            m_nodes.end(),
            [&name](const RigNode& j) {
                return j.m_name == name;
            });
        return it != m_nodes.end() ? &m_nodes[it->m_index] : nullptr;
    }

    animation::Joint* Rig::registerJoint(
        const aiBone* bone)
    {
        std::string nodeName = assimp_util::normalizeName(bone->mName);
        auto* rigNode = findNode(nodeName);

        if (!rigNode) {
            //throw fmt::format("rig_node_not_found: joint={}", nodeName);
            KI_ERROR(fmt::format(
                "rig_node_not_found: rig={}, joint={}",
                m_name, nodeName));
            //return nullptr;
        }

        if (rigNode) {
            rigNode->m_hasJoint = true;
        }

        return &m_jointContainer.registerJoint(bone, rigNode ? rigNode->m_index : -1);
    }

    int16_t Rig::registerSocket(const animation::RigSocket& a_socket)
    {
        const auto& jointName = a_socket.m_jointName;

        // NOTE KI allow duplicate sockets in same joint
        // => they can have different offset/rotation
        if (false)
        {
            const auto& socketIt = m_NameToSocket.find(jointName);
            if (socketIt != m_NameToSocket.end()) {
                const auto& old = m_sockets[socketIt->second];
                if (old.m_name != a_socket.m_name)
                    throw std::runtime_error(fmt::format(
                        "RIG_ERROR: duplicate_socket: rig={}, joint={}, socket={} vs. {}",
                        m_name, old.m_jointName, old.m_name, a_socket.m_name));
                return socketIt->second;
            }
        }

        const auto& nodeIt = std::find_if(
            m_nodes.begin(),
            m_nodes.end(),
            [&jointName](const auto& joint) { return joint.m_name == jointName; });
        if (nodeIt == m_nodes.end()) return -1;

        auto& rigNode = *nodeIt;

        int16_t socketIndex = static_cast<int16_t>(m_sockets.size());
        {
            m_sockets.push_back(a_socket);
            auto& socket = m_sockets[socketIndex];
            socket.m_nodeIndex = rigNode.m_index;
            socket.m_index = socketIndex;
        }
        rigNode.m_hasSocket = true;
        rigNode.m_socketIndex = socketIndex;
        m_NameToSocket.insert({ rigNode.m_name, socketIndex });

        const auto& line = fmt::format(
            "RIG_SOCKET_ADD: {} - {}.{}, node={}, socket={}.{}]",
            m_name,
            rigNode.m_parentIndex,
            rigNode.m_index,
            rigNode.m_name,
            socketIndex,
            a_socket.m_name);

        return socketIndex;
    }

    const animation::RigSocket* Rig::getSocket(int16_t socketIndex) const noexcept
    {
        return socketIndex >= 0 ? &m_sockets[socketIndex] : nullptr;
    }

    animation::RigSocket* Rig::modifySocket(int16_t socketIndex) noexcept
    {
        return socketIndex >= 0 ? &m_sockets[socketIndex] : nullptr;
    }

    const animation::RigSocket* Rig::findSocket(const std::string& socketName) const noexcept
    {
        const auto& it = std::find_if(
            m_sockets.begin(),
            m_sockets.end(),
            [&socketName](const auto& socket) { return socketName == socket.m_name; });

        if (it == m_sockets.end()) return nullptr;

        return &(*it);
    }

    std::vector<std::string> Rig::getSocketNames() const noexcept
    {
        std::vector<std::string> names;

        for (auto& socket : m_sockets) {
            names.push_back(socket.m_name);
        }
        return names;
    }

    void Rig::registerMesh(
        uint16_t nodeIndex,
        mesh::ModelMesh* mesh)
    {
        const auto& it = m_nodeMeshes.find(nodeIndex);
        if (it == m_nodeMeshes.end()) {
            m_nodeMeshes.insert({ nodeIndex, {} });
        }
        m_nodeMeshes[nodeIndex].emplace_back(mesh);
    }

    void Rig::dump() const
    {
        KI_INFO_OUT(fmt::format(
            "\n=======================\n[RIG SUMMARY: {} ({}) - {} joints]\nHIERARCHY:\n{}\nANIMATIONS:\n{}\nSOCKETS:\n{}\n=======================",
            m_name,
            m_skeletonRootNodeName,
            m_jointContainer.size(),
            getHierarchySummary(0),
            getAnimationSummary(0),
            getSocketSummary(0)));
    }

    std::string Rig::getHierarchySummary(int16_t level) const
    {
        std::string sb;
        sb.reserve(10000);

        auto appendLine = [](auto& sb, auto level, const auto& line) {
            for (int i = 0; i < level; i++) {
                sb += "   ";
            }
            sb += line;
            sb += "\n";
            };

        for (const auto& rigNode : m_nodes) {
            const RigSocket* socket = rigNode.m_socketIndex >= 0 ? &m_sockets[rigNode.m_socketIndex] : nullptr;

            const auto& line = fmt::format(
                "NODE: [{}{}.{}, name={}{}]",
                rigNode.m_hasJoint ? "+" : "-",
                rigNode.m_parentIndex,
                rigNode.m_index,
                rigNode.m_name,
                //rigNode.m_hasJoint ? fmt::format(", joint={}", rigNode.m_socketIndex) : "",
                socket ? fmt::format(", socket={}.{}", socket->m_index, socket->m_name) : "");

            const auto& line2 = rigNode.m_transform == glm::mat4{ 1.f }
                ? "TRAN: [ID]"
                : fmt::format(
                    "TRAN: {}",
                    rigNode.m_transform);

            appendLine(sb, rigNode.m_level, line);
            appendLine(sb, rigNode.m_level, line2);

            if (const auto& it = m_nodeMeshes.find(rigNode.m_index); it != m_nodeMeshes.end()) {
                std::vector<std::string> meshLines;

                for (const auto& mi : it->second) {
                    std::string meshLine = fmt::format(
                        "MATE: [mesh={}, material={}, vertices={}, indeces={}]",
                        mi.m_name, mi.m_material, mi.m_vertexCount, mi.m_indexCount);

                    appendLine(sb, rigNode.m_level, meshLine);
                }
            }
        }

        return sb;
    }

    std::string Rig::getAnimationSummary(int16_t level) const
    {
        std::string sb;

        auto appendLine = [](auto& sb, auto level, const auto& line) {
            for (int i = 0; i < level; i++) {
                sb += "    ";
            }
            sb += line;
            sb += "\n";
            };

        for (const auto& anim : m_clipContainer.m_animations) {
            const auto& line = fmt::format(
                "ANIM: [{}, name={}] - duration={}, tps={}, clips={}, channels={}",
                anim->m_index,
                anim->m_uniqueName,
                anim->m_duration,
                anim->m_ticksPerSecond,
                anim->m_clipCount,
                anim->m_channels.size());
            appendLine(sb, 0, line);
        }

        for (const auto& clip : m_clipContainer.m_clips) {
            const auto& line = fmt::format(
                "CLIP: [{}, {} / {}] - range=[{}, {}], duration={}, loop={}, anim={}.{}",
                clip.m_index,
                clip.getName(),
                clip.m_uniqueName,
                clip.m_firstFrame,
                clip.m_lastFrame,
                clip.m_durationSecs,
                clip.m_loop,
                clip.m_animationIndex,
                clip.m_animationName);
            appendLine(sb, 0, line);
        }

        return sb;
    }

    std::string Rig::getSocketSummary(int16_t level) const
    {
        std::string sb;

        auto appendLine = [](auto& sb, auto level, const auto& line) {
            for (int i = 0; i < level; i++) {
                sb += "    ";
            }
            sb += line;
            sb += "\n";
            };

        for (const auto& socket : m_sockets) {
            const auto& line = fmt::format(
                "SOCK: {}.{}, joint={}, pos={}, rot={}, scale={}, meshScale={}, nodeIndex={}",
                socket.m_index,
                socket.m_name,
                socket.m_jointName,
                socket.m_offset.m_position,
                socket.m_offset.m_rotation,
                socket.m_offset.m_scale,
                socket.m_meshScale,
                socket.m_nodeIndex
            );
            appendLine(sb, 0, line);
        }

        return sb;
    }
}
