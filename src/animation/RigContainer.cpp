#include "RigContainer.h"

#include <tuple>

#include <assimp/scene.h>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/util.h"

#include "Animation.h"
#include "RigJoint.h"
#include "BoneInfo.h"
#include "VertexBone.h"

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
        m_jointPrefixes.push_back("Scavenger_ ");
        m_jointPrefixes.push_back("Scavenger_");
        m_jointPrefixes.push_back("humanoid_ ");
        m_jointPrefixes.push_back("humanoid_");
    }

    RigContainer::~RigContainer() = default;

    animation::RigJoint& RigContainer::addJoint(const aiNode* node)
    {
        auto& rigJoint = m_joints.emplace_back(node);
        rigJoint.m_index = static_cast<int16_t>(m_joints.size() - 1);

        const auto [foundAlias, alias] = reolveJointAlias(rigJoint.m_name, m_jointPrefixes);
        if (foundAlias) {
            rigJoint.m_hasAliasName = true;
            rigJoint.m_aliasName = alias;
        }

        return rigJoint;
    }

    animation::BoneInfo& RigContainer::registerBone(const aiBone* bone) noexcept
    {
        auto& bi = m_boneContainer.registerBone(bone);
        auto* rigJoint = findJoint(bi.m_jointName);

        assert(rigJoint);

        if (bi.m_jointIndex >= 0) {
            assert(rigJoint->m_index == bi.m_jointIndex);
            return bi;
        }

        if (rigJoint) {
            m_boneContainer.bindJoint(bi.m_index, rigJoint->m_index);
        }

        return bi;
    }

    const animation::RigJoint* RigContainer::getJoint(int16_t index) const noexcept
    {
        return &m_joints[index];
    }

    const animation::RigJoint* RigContainer::findJoint(const std::string& name) const noexcept
    {
        const auto [foundAlias, alias] = reolveJointAlias(name, m_jointPrefixes);

        const auto& it = std::find_if(
            m_joints.begin(),
            m_joints.end(),
            [&name, &foundAlias, &alias](const RigJoint& j) {
                if (j.m_name == name) return true;
                if (j.m_hasAliasName && foundAlias) {
                    return j.m_aliasName == alias;
                }
                return false;
            });
        return it != m_joints.end() ? &m_joints[it->m_index] : nullptr;
    }

    bool RigContainer::hasBones() const noexcept
    {
        return m_boneContainer.hasBones();
    }

    int16_t RigContainer::registerSocket(const animation::RigSocket& a_socket)
    {
        const auto& jointName = a_socket.m_jointName;

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

        const auto& jointIt = std::find_if(
            m_joints.begin(),
            m_joints.end(),
            [&jointName](const auto& joint) { return joint.m_name == jointName; });
        if (jointIt == m_joints.end()) return -1;

        auto& rigJoint = *jointIt;

        int16_t index = static_cast<int16_t>(m_sockets.size());
        {
            m_sockets.push_back(a_socket);
            auto& socket = m_sockets[index];
            socket.m_jointIndex = rigJoint.m_index;
            socket.m_index = index;
        }
        rigJoint.m_socketIndex = index;
        m_NameToSocket.insert({ rigJoint.m_name, index });

        const auto& line = fmt::format(
            "RIG_SOCKET_ADD: {} - {}.{}, joint={}, bone={}, socket={}.{}]",
            m_name,
            rigJoint.m_parentIndex,
            rigJoint.m_index,
            rigJoint.m_name,
            rigJoint.m_boneIndex,
            index,
            a_socket.m_name);

        return index;
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
        const auto& it = m_jointMeshes.find(jointIndex);
        if (it != m_jointMeshes.end()) {
            m_jointMeshes.insert({ jointIndex, {} });
        }
        m_jointMeshes[jointIndex].emplace_back(mesh);
    }

    void RigContainer::prepare()
    {
        validate();

        // Mark bones required for rigged animation
        // - all joints with bone
        // - all parent joints of bone
        for (auto& rigJoint : m_joints) {
            auto* bone = m_boneContainer.findByJointIndex(rigJoint.m_index);
            if (!bone) continue;

            rigJoint.m_boneRequired = true;
            rigJoint.m_boneIndex = bone->m_index;

            for (auto jointIndex = rigJoint.m_parentIndex; jointIndex >= 0;) {
                auto& parent = m_joints[jointIndex];
                if (parent.m_boneRequired) break;
                parent.m_boneRequired = true;
                jointIndex = parent.m_parentIndex;
            }
        }

        // NOTE KI mesh required for calculating transforms for attached meshes
        for (auto& rigJoint : m_joints) {
            const auto& it = std::find_if(
                m_sockets.begin(),
                m_sockets.end(),
                [&rigJoint](const auto& socket) { return socket.m_jointIndex == rigJoint.m_index; });
            if (it == m_sockets.end()) continue;

            rigJoint.m_socketRequired = true;

            for (auto jointIndex = rigJoint.m_parentIndex; jointIndex >= 0;) {
                auto& parent = m_joints[jointIndex];
                // NOTE KI m_sockets is not sorted
                //if (parent.m_socketRequired) break;
                parent.m_socketRequired = true;
                jointIndex = parent.m_parentIndex;
            }
        }

        for (auto& rigJoint : m_joints) {
            if (rigJoint.m_boneIndex >= 0) {
                const auto& bi = m_boneContainer.m_boneInfos[rigJoint.m_boneIndex];

                KI_INFO_OUT(fmt::format(
                    "PREPARE: name={}\nbone: {}\njoin: {}",
                    rigJoint.m_name,
                    bi.m_offsetMatrix,
                    rigJoint.m_globalInvTransform));
            }
            else {
                KI_INFO_OUT(fmt::format(
                    "PREPARE: name={}\nbone: {}\njoin: {}",
                    rigJoint.m_name,
                    "NA",
                    rigJoint.m_globalInvTransform));
            }
        }
    }

    void RigContainer::validate() const
    {
        // NOTE KI check that all bones are related to some joint
        // - every bone has joint
        // - not every joint has bone
        for (const auto& it : m_boneContainer.m_jointNameToIndex) {
            const auto& name = it.first;

            const auto& jointIt = std::find_if(
                m_joints.begin(),
                m_joints.end(),
                [&name](const auto& rigJoint) {
                    return rigJoint.m_name == name;
                });

            if (jointIt == m_joints.end()) throw std::runtime_error(fmt::format("missing_bone_joint: {}", name));
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

        for (const auto& rigJoint : m_joints) {
            const RigSocket* socket = rigJoint.m_socketIndex >= 0 ? &m_sockets[rigJoint.m_socketIndex] : nullptr;

            const auto& line = fmt::format(
                "JOIN: [{}{}.{}, name={}{}{}{}]",
                rigJoint.m_boneRequired ? "+" : "-",
                rigJoint.m_parentIndex,
                rigJoint.m_index,
                rigJoint.m_name,
                rigJoint.m_hasAliasName ? fmt::format(", alias={}", rigJoint.m_aliasName) : "",
                rigJoint.m_boneIndex >= 0 ? fmt::format(", bone={}", rigJoint.m_boneIndex) : "",
                socket ? fmt::format(", socket={}.{}", socket->m_index, socket->m_name) : "");

            const auto& line2 = rigJoint.m_transform == glm::mat4{ 1.f }
                    ? "TRAN: [ID]"
                    : fmt::format(
                        "TRAN: {}",
                        rigJoint.m_transform);

            appendLine(sb, rigJoint.m_level, line);
            appendLine(sb, rigJoint.m_level, line2);

            if (const auto& it = m_jointMeshes.find(rigJoint.m_index); it != m_jointMeshes.end()) {
                std::vector<std::string> meshLines;

                for (const auto& mi : it->second) {
                    std::string meshLine = fmt::format(
                        "MATE: [mesh={}, material={}, vertices={}, indeces={}]",
                        mi.m_name, mi.m_material, mi.m_vertexCount, mi.m_indexCount);

                    appendLine(sb, rigJoint.m_level, meshLine);
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
                "CLIP: [{}, {}] - range=[{}, {}], duration={}, loop={}, anim={}.{}",
                clip.m_index,
                clip.m_name,
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
                "SOCK: {}.{}, joint={}, offset={}, rot={}, scale={}, meshScale={}, jointIndex={}",
                socket.m_index,
                socket.m_name,
                socket.m_jointName,
                socket.m_offset,
                socket.m_rotation,
                socket.m_scale,
                socket.m_meshScale,
                socket.m_jointIndex
                );
            appendLine(sb, 0, line);
        }

        return sb;
    }

    //void RigContainer::calculateInvTransforms() noexcept
    //{
    //    for (auto& rigJoint : m_joints) {
    //        rigJoint.m_globalInvTransform = glm::inverse(rigJoint.m_globalTransform);
    //    }
    //}
}
