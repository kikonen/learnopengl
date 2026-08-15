#include "Mesh.h"

#include "util/debug.h"

#include <fmt/format.h>

#include "animation/Rig.h"
#include "animation/JointContainer.h"

#include "pool/IdGenerator.h"

#include "registry/VaoRegistry.h"

namespace {
    IdGenerator<ki::mesh_id> ID_GENERATOR;

    static const util::Ref<animation::Rig> EMPTY_RIG;
    static const util::Ref<animation::JointContainer> EMPTY_JOINT_CONTAINER;
}

namespace mesh {
    Mesh::Mesh(std::string_view name)
        : m_id{ ID_GENERATOR.nextId() },
        m_name{ name }
    {
    }

    Mesh::~Mesh()
    {
        //KI_INFO(fmt::format("MESH: delete {}", str()));
    }

    std::string Mesh::str() const noexcept
    {
        //return fmt::format(
        //    "<MESH: id={}, name={}, alias={}, baseVertex={}, baseIndex={}, vertexCount={}, indexCount={}>",
        //    m_id,
        //    m_name,
        //    m_alias,
        //    getBaseVertex(),
        //    getBaseIndex(),
        //    getDefinedVertexCount(),
        //    getDefinedIndexCount());
        return fmt::format(
            "<MESH: id={}, name={}, alias={}, baseVertex={}, vertexCount={}, indexCount={}>",
            m_id,
            m_name,
            m_alias,
            getBaseVertex(),
            getBaseIndex(),
            getIndexCount());
    }

    const util::Ref<animation::Rig>& Mesh::getRig() const
    {
        return EMPTY_RIG;
    }

    const util::Ref<animation::JointContainer>& Mesh::getJointContainer() const
    {
        return EMPTY_JOINT_CONTAINER;
    }

    void Mesh::setMaterial(const util::Ref<Material>& src) noexcept
    {
        if (!src) {
            m_material.reset();
            return;
        }

        if (!m_material) {
            m_material = util::Ref<Material>::create();
        }
        *m_material = *src;
    }

    const kigl::GLVertexArray* Mesh::prepareVAO()
    {
        return setupVAO(VaoRegistry::get().getTexturedVao(), true);
    }
}
