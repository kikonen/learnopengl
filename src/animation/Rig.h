#pragma once

#include <string>
#include <vector>

#include "ClipContainer.h"
#include "MeshInfo.h"
#include "RigSocket.h"
#include "JointContainer.h"

namespace mesh
{
    class ModelMesh;
}

namespace mesh_set
{
    struct Skeleton;
    class AssimpImporter;
    class AnimationImporter;
    class RigNodeTreeGenerator;
}

struct aiBone;

namespace animation
{
    struct RigNode;
    struct Joint;

    // Rigged skeleton consisting of 
    // - skeleton nodes (all nodes under specific root node)
    // - joints
    // 
    // NOTE KI each separate set of joints with same skeleton root are same Rig
    struct Rig
    {
        friend struct mesh_set::Skeleton;
        friend class mesh_set::AssimpImporter;
        friend class mesh_set::AnimationImporter;
        friend class mesh_set::RigNodeTreeGenerator;
        friend class AnimationSystem;
        friend class AnimateNode;
        friend class Animator;

        bool empty() const noexcept
        {
            return m_nodes.empty();
        }

        const std::string& getName() const noexcept
        {
            return m_name;
        }

        // finalizes container; no changes after this
        void prepare();
        void prepareSockets();

        void validate() const;

        const std::vector<animation::RigSocket>& getSockets() const noexcept
        {
            return m_sockets;
        }

        const animation::RigNode* getRoot() const noexcept
        {
            return getNode(0);
        }

        const animation::RigNode* getNode(int16_t index) const noexcept;

        const animation::RigNode* findNode(const std::string& name) const noexcept;
        animation::RigNode* findNode(const std::string& name) noexcept;

        animation::Joint* registerJoint(
            const aiBone* bone);

        // @return registered index in m_sockets, -1 if joint not found
        int16_t registerSocket(const animation::RigSocket& socket);

        // @return registered socket, null if not found
        const animation::RigSocket* getSocket(int16_t socketIndex) const noexcept;

        // @return registered socket, null if not found
        animation::RigSocket* modifySocket(int16_t socketIndex) noexcept;

        const animation::RigSocket* findSocket(const std::string& socketName) const noexcept;

        std::vector<std::string> getSocketNames() const noexcept;

        bool hasSockets() const noexcept
        {
            return !m_sockets.empty();
        }

        const animation::ClipContainer& getClipContainer() const noexcept
        {
            return m_clipContainer;
        }

        animation::ClipContainer& modifyClipContainer() noexcept
        {
            return m_clipContainer;
        }

        const animation::JointContainer& getJointContainer() const noexcept
        {
            return m_jointContainer;
        }

        // NOTE KI for debug
        void registerMesh(
            uint16_t nodeIndex,
            mesh::ModelMesh* mesh);

        void dump() const;
        std::string getHierarchySummary(int16_t level) const;
        std::string getAnimationSummary(int16_t level) const;
        std::string getSocketSummary(int16_t level) const;

    private:
        std::string m_name;
        // skeleton_root = common ancestor of joints in rig
        std::string m_skeletonRootNodeName;
        std::vector<animation::RigNode> m_nodes;

        std::vector<animation::RigSocket> m_sockets;
        std::map<std::string, uint16_t> m_NameToSocket;

        // Animations bound to this rig
        animation::ClipContainer m_clipContainer;

        // NOTE KI for debug
        // { nodeIndex: [mesh, ...] }
        std::map<uint16_t, std::vector<MeshInfo>> m_nodeMeshes;

        // TODO KI need to change logic so that mesh_set can share
        // same rig (and thus animate nodes only ONCE), but have separate
        // joint containers *per mesh*
        // => Makes huge difference especially with LODx meshes sharing same skeleton
        animation::JointContainer m_jointContainer;
    };
}
