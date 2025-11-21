#include "RigContainer.h"

#include <tuple>

#include <assimp/scene.h>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/util.h"

#include "Animation.h"
#include "RigNode.h"
#include "Joint.h"
#include "VertexJoint.h"

namespace {
    std::pair<bool, std::string> reolveJointAlias(
        const std::string& name,
        std::vector<std::string> jointPrefixes)
    {
        for (const auto& prefix : jointPrefixes) {
            if (util::startsWith(name, prefix)) {
                const auto& alias = prefix.size() != name.size()
                    ? name.substr(prefix.size(), name.size() - prefix.size())
                    : "<ROOT>";
                return { true, alias };
                break;
            }
        }
        return { false, "" };
    }
}

namespace animation {
    RigContainer::RigContainer(const std::string& name)
        : m_name{ name }
    {
        // TODO KI WTF IS THIS HACK!!!
        m_nodePrefixes.push_back("Scavenger_ ");
        m_nodePrefixes.push_back("Scavenger_");
        m_nodePrefixes.push_back("humanoid_ ");
        m_nodePrefixes.push_back("humanoid_");
    }

    RigContainer::~RigContainer() = default;

    animation::RigNode& RigContainer::addNode(const aiNode* node)
    {
        assert(!m_prepared);

        auto& rigNode = m_nodes.emplace_back(node);
        rigNode.m_index = static_cast<int16_t>(m_nodes.size() - 1);

        const auto [foundAlias, alias] = reolveJointAlias(rigNode.m_name, m_nodePrefixes);
        if (foundAlias) {
            rigNode.m_hasAliasName = true;
            rigNode.m_aliasName = alias;
        }

        return rigNode;
    }

    animation::Joint& RigContainer::registerJoint(const aiBone* bone) noexcept
    {
        assert(!m_prepared);

        auto& bi = m_jointContainer.registerJoint(bone);
        auto* rigNode = findNode(bi.m_nodeName);

        assert(rigNode);

        if (bi.m_nodeIndex >= 0) {
            assert(rigNode->m_index == bi.m_nodeIndex);
            return bi;
        }

        if (rigNode) {
            m_jointContainer.bindNode(bi.m_index, rigNode->m_index);
        }

        return bi;
    }

    const animation::RigNode* RigContainer::getNode(int16_t nodeIndex) const noexcept
    {
        return &m_nodes[nodeIndex];
    }

    const animation::RigNode* RigContainer::findNode(const std::string& name) const noexcept
    {
        const auto [foundAlias, alias] = reolveJointAlias(name, m_nodePrefixes);

        const auto& it = std::find_if(
            m_nodes.begin(),
            m_nodes.end(),
            [&name, &foundAlias, &alias](const RigNode& j) {
                if (j.m_name == name) return true;
                if (j.m_hasAliasName && foundAlias) {
                    return j.m_aliasName == alias;
                }
                return false;
            });
        return it != m_nodes.end() ? &m_nodes[it->m_index] : nullptr;
    }

    bool RigContainer::hasJoints() const noexcept
    {
        return m_jointContainer.hasJoints();
    }

    int16_t RigContainer::registerSocket(const animation::RigSocket& a_socket)
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
        rigNode.m_socketIndex = socketIndex;
        m_NameToSocket.insert({ rigNode.m_name, socketIndex });

        const auto& line = fmt::format(
            "RIG_SOCKET_ADD: {} - {}.{}, node={}, joint={}, socket={}.{}]",
            m_name,
            rigNode.m_parentIndex,
            rigNode.m_index,
            rigNode.m_name,
            rigNode.m_jointIndex,
            socketIndex,
            a_socket.m_name);

        return socketIndex;
    }

    const animation::RigSocket* RigContainer::getSocket(int16_t socketIndex) const noexcept
    {
        return socketIndex >= 0 ? &m_sockets[socketIndex] : nullptr;
    }

    animation::RigSocket* RigContainer::modifySocket(int16_t socketIndex) noexcept
    {
        return socketIndex >= 0 ? &m_sockets[socketIndex] : nullptr;
    }

    const animation::RigSocket* RigContainer::findSocket(const std::string& socketName) const noexcept
    {
        const auto& it = std::find_if(
            m_sockets.begin(),
            m_sockets.end(),
            [&socketName](const auto& socket) { return socketName == socket.m_name; });

        if (it == m_sockets.end()) return nullptr;

        return &(*it);
    }

    std::vector<std::string> RigContainer::getSocketNames() const noexcept
    {
        std::vector<std::string> names;

        for (auto& socket : m_sockets) {
            names.push_back(socket.m_name);
        }
        return names;
    }

    void RigContainer::registerMesh(
        uint16_t jointIndex,
        mesh::ModelMesh* mesh)
    {
        const auto& it = m_nodeMeshes.find(jointIndex);
        if (it != m_nodeMeshes.end()) {
            m_nodeMeshes.insert({ jointIndex, {} });
        }
        m_nodeMeshes[jointIndex].emplace_back(mesh);
    }

    void RigContainer::prepare()
    {
        if (m_prepared) return;
        m_prepared = true;

        //m_jointContainer.sort();

        validate();

        // Mark nodes required for rigged animation joints
        // - all nodes with joint
        // - all parent nodes of joint
        for (auto& rigNode : m_nodes) {
            auto* joint = m_jointContainer.findByNodeIndex(rigNode.m_index);
            if (!joint) continue;

            rigNode.m_jointRequired = true;
            rigNode.m_jointIndex = joint->m_index;

            for (auto nodeIndex = rigNode.m_parentIndex; nodeIndex >= 0;) {
                auto& parent = m_nodes[nodeIndex];
                if (parent.m_jointRequired) break;
                parent.m_jointRequired = true;
                nodeIndex = parent.m_parentIndex;
            }
        }

        // NOTE KI mesh required for calculating transforms for attached meshes
        for (auto& rigNode : m_nodes) {
            const auto& it = std::find_if(
                m_sockets.begin(),
                m_sockets.end(),
                [&rigNode](const auto& socket) { return socket.m_nodeIndex == rigNode.m_index; });
            if (it == m_sockets.end()) continue;

            rigNode.m_socketRequired = true;

            for (auto jointIndex = rigNode.m_parentIndex; jointIndex >= 0;) {
                auto& parent = m_nodes[jointIndex];
                // NOTE KI m_sockets is not sorted
                //if (parent.m_socketRequired) break;
                parent.m_socketRequired = true;
                jointIndex = parent.m_parentIndex;
            }
        }

        for (auto& rigNode : m_nodes) {
            if (rigNode.m_jointIndex >= 0) {
                const auto& joint = m_jointContainer.m_joints[rigNode.m_jointIndex];

                KI_INFO_OUT(fmt::format(
                    "PREPARE: name={}\njoin: {}\nnode: {}",
                    rigNode.m_name,
                    joint.m_offsetMatrix,
                    rigNode.m_globalInvTransform));
            }
            else {
                KI_INFO_OUT(fmt::format(
                    "PREPARE: name={}\njoin: {}\nnode: {}",
                    rigNode.m_name,
                    "NA",
                    rigNode.m_globalInvTransform));
            }
        }
    }

    void RigContainer::validate() const
    {
        // NOTE KI check that all joints are related to some node
        // - every joint has node
        // - not every node has joint
        for (const auto& it : m_jointContainer.m_nodeNameToIndex) {
            const auto& name = it.first;

            const auto& nodeIt = std::find_if(
                m_nodes.begin(),
                m_nodes.end(),
                [&name](const auto& rigNode) {
                    return rigNode.m_name == name;
                });

            if (nodeIt == m_nodes.end()) throw std::runtime_error(fmt::format("missing_joint_node: {}", name));
        }
    }

    void RigContainer::dump() const
    {
        KI_INFO_OUT(fmt::format(
            "\n=======================\n[RIG SUMMARY: {}]\nHIERARCHY:\n{}\nANIMATIONS:\n{}\nSOCKETS:\n{}\n=======================",
            m_name,
            getHierarchySummary(0),
            getAnimationSummary(0),
            getSocketSummary(0)));
    }

    std::string RigContainer::getHierarchySummary(int16_t level) const
    {
        std::string sb;
        sb.reserve(10000);

        auto appendLine = [](auto& sb, auto level, const auto& line) {
            for (int i = 0; i < level; i++) {
                sb += "    ";
            }
            sb += line;
            sb += "\n";
            };

        for (const auto& rigNode : m_nodes) {
            const RigSocket* socket = rigNode.m_socketIndex >= 0 ? &m_sockets[rigNode.m_socketIndex] : nullptr;

            const auto& line = fmt::format(
                "NODE: [{}{}.{}, name={}{}{}{}]",
                rigNode.m_jointRequired ? "+" : "-",
                rigNode.m_parentIndex,
                rigNode.m_index,
                rigNode.m_name,
                rigNode.m_hasAliasName ? fmt::format(", alias={}", rigNode.m_aliasName) : "",
                rigNode.m_jointIndex >= 0 ? fmt::format(", joint={}", rigNode.m_jointIndex) : "",
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

    std::string RigContainer::getAnimationSummary(int16_t level) const
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

    std::string RigContainer::getSocketSummary(int16_t level) const
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

    //void RigContainer::calculateInvTransforms() noexcept
    //{
    //    for (auto& rigNode : m_joints) {
    //        rigNode.m_globalInvTransform = glm::inverse(rigNode.m_globalTransform);
    //    }
    //}
}
