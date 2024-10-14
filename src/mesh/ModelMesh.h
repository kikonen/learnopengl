#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "animation/VertexBone.h"

#include "mesh/VaoMesh.h"
#include "mesh/Index.h"
#include "mesh/Vertex.h"

namespace animation {
    struct RigContainer;
    struct VertexBone;
}

namespace mesh {
    class ModelMesh final : public VaoMesh {
        friend class ModelLoader;
        friend class AssimpLoader;

    public:
        ModelMesh(
            std::string_view name);

        virtual ~ModelMesh();

        virtual const kigl::GLVertexArray* prepareVAO() override;
        virtual const kigl::GLVertexArray* setupVAO(mesh::TexturedVAO* vao, bool shared) override;

        virtual void prepareLodMesh(
            mesh::LodMesh& lodMesh);

        virtual backend::DrawOptions::Mode getDrawMode() override;

        virtual std::shared_ptr<animation::RigContainer> getRigContainer() override
        {
            return m_rig;
        }

    public:
        std::vector<animation::VertexBone> m_vertexBones;

        std::shared_ptr<animation::RigContainer> m_rig;
    };
}
