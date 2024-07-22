#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "animation/VertexBone.h"

#include "mesh/Index.h"
#include "mesh/Vertex.h"
#include "mesh/Mesh.h"

namespace animation {
    struct RigContainer;
    struct VertexBone;
}

namespace mesh {
    class ModelMesh final : public Mesh {
        friend class ModelLoader;
        friend class AssimpLoader;

    public:
        ModelMesh(
            std::string_view name);

        virtual ~ModelMesh();

        virtual const kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) override;

        virtual const kigl::GLVertexArray* prepareRTDebug(
            const PrepareContext& ctx) override;

        virtual void prepareLodMesh(
            mesh::LodMesh& lodMesh);

        virtual std::shared_ptr<animation::RigContainer> getRigContainer() override
        {
            return m_rig;
        }

    public:
        std::vector<animation::VertexBone> m_vertexBones;

        std::shared_ptr<animation::RigContainer> m_rig;
    };
}
