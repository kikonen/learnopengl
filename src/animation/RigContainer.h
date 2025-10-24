#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "ki/size.h"

#include "material/Material.h"

#include "RigNode.h"
#include "RigSocket.h"
#include "MeshInfo.h"
#include "Clip.h"

#include "JointContainer.h"
#include "ClipContainer.h"

struct aiNode;
struct aiBone;

namespace mesh {
    class ModelMesh;
}

namespace animation {
    struct RigNode;

    struct RigContainer {
        RigContainer(const std::string& name);
        ~RigContainer();

        animation::RigNode& addNode(const aiNode* node);

        // Register new joint or return old
        animation::Joint& registerJoint(const aiBone* bone) noexcept;

        // @return true if rig is empty (thus no joints, and thus rig is not needed)
        bool empty() const noexcept
        {
            return m_jointContainer.empty();
        }

        const animation::RigNode* getNode(int16_t index) const noexcept;

        // @return nullptr if not found
        const animation::RigNode* findNode(const std::string& name) const noexcept;

        bool hasJoints() const noexcept;

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

        // NOTE KI for debug
        void registerMesh(
            uint16_t jointIndex,
            mesh::ModelMesh* mesh);

        void prepare();
        void validate() const;

        void dump() const;
        std::string getHierarchySummary(int16_t level) const;
        std::string getAnimationSummary(int16_t level) const;
        std::string getSocketSummary(int16_t level) const;

        //void calculateInvTransforms() noexcept;

        const std::string m_name;

        std::vector<animation::RigNode> m_nodes;

        JointContainer m_jointContainer;

        std::vector<animation::RigSocket> m_sockets;

        std::map<std::string, uint16_t> m_NameToSocket;

        animation::ClipContainer m_clipContainer;

        // NOTE KI for debug
        std::map<uint16_t, std::vector<MeshInfo>> m_nodeMeshes;

        std::vector<std::string> m_nodePrefixes;
    };
}
