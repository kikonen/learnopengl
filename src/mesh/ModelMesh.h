#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "util/Ref.h"

#include "animation/VertexJoint.h"

#include "mesh/VaoMesh.h"
#include "mesh/Index.h"
#include "mesh/Vertex.h"

namespace animation {
    struct Rig;
    struct JointContainer;
    struct VertexJoint;
}

namespace mesh_set
{
    class ModelImporter;
    class AssimpImporter;

    namespace decoder
    {
        class ModelMeshDecoder;
    }
}

namespace loader
{
    class NodeTypeBuilder;
}

namespace mesh
{
    class ModelMesh final : public VaoMesh
    {
        friend class mesh_set::ModelImporter;
        friend class mesh_set::AssimpImporter;
        friend class mesh_set::decoder::ModelMeshDecoder;
        friend class loader::NodeTypeBuilder;

    public:
        ModelMesh(
            std::string_view name);

        virtual ~ModelMesh();

        const kigl::GLVertexArray* prepareVAO() override;
        const kigl::GLVertexArray* setupVAO(mesh::TexturedVAO* vao, bool shared) override;

        const util::Ref<animation::Rig>& getRig() const override
        {
            return m_rig;
        }

        const glm::mat4& getRigBaseTransform() const override
        {
            return m_rigBaseTransform;
        }

        const util::Ref<animation::JointContainer>& getJointContainer() const override
        {
            return m_jointContainer;
        }

        const std::vector<animation::VertexJoint>& getVertexJoints() const noexcept
        {
            return m_vertexJoints;
        }

        std::vector<animation::VertexJoint>& modifyVertexJoints() noexcept
        {
            return m_vertexJoints;
        }

    private:
        std::vector<animation::VertexJoint> m_vertexJoints;

    private:
        // shared in MeshSet
        util::Ref<animation::Rig> m_rig;

        // owned by this Mesh
        util::Ref<animation::JointContainer> m_jointContainer;

        glm::mat4 m_rigBaseTransform{ 1.f };
    };
}
