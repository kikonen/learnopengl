#include "ModelMesh.h"

#include <glm/glm.hpp>

#include <string>
#include <algorithm>

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "asset/Sphere.h"

#include "mesh/LodMesh.h"

#include "animation/RigContainer.h"

#include "mesh/vao/TexturedVAO.h"
#include "mesh/vao/SkinnedVAO.h"

#include "mesh/vao/PositionEntry.h"

#include "engine/PrepareContext.h"
#include "registry/VaoRegistry.h"


namespace mesh {
    ModelMesh::ModelMesh(
        std::string_view name)
        : VaoMesh{ name }
    {
    }

    ModelMesh::~ModelMesh()
    {
        KI_INFO(fmt::format("MODEL_MESH: delete - {}", str()));
        m_vertices.clear();
    }

    const kigl::GLVertexArray* ModelMesh::prepareVAO()
    {
        TexturedVAO* vao;

        if (m_rig && !m_vertexBones.empty()) {
            vao = VaoRegistry::get().getSkinnedVao();
        }
        else {
            vao = VaoRegistry::get().getTexturedVao();
        }

        return setupVAO(vao, true);
    }

    const kigl::GLVertexArray* ModelMesh::setupVAO(mesh::TexturedVAO* vao, bool shared)
    {
        if (shared) {
            if (m_preparedVAO) return m_vao;
            m_preparedVAO = true;
        }

        m_vboIndex = vao->reserveVertices(m_vertices.size());
        m_eboIndex = vao->reserveIndeces(m_indeces.size());

        m_vertexCount = static_cast<uint32_t>(m_vertices.size());
        m_indexCount = static_cast<uint32_t>(m_indeces.size());

        SkinnedVAO* skinnedVao = dynamic_cast<mesh::SkinnedVAO*>(vao);

        vao->updateVertices(
            m_vboIndex,
            m_vertices);

        vao->updateIndeces(
            m_eboIndex,
            m_indeces);

        if (skinnedVao) {
            skinnedVao->reserveVertexBones(m_vertexBones.size());
            skinnedVao->updateVertexBones(
                m_vboIndex,
                m_vertexBones);
        }

        auto* va = vao->getVAO();
        if (shared) {
            m_vao = va;
        }
        return va;
    }

    void ModelMesh::prepareLodMesh(
        mesh::LodMesh& lodMesh)
    {
        lodMesh.m_baseVertex = getBaseVertex();
        lodMesh.m_baseIndex = getBaseIndex();
        lodMesh.m_indexCount = getIndexCount();

        auto& drawOptions = lodMesh.m_drawOptions;
        drawOptions.m_type = backend::DrawOptions::Type::elements;
        drawOptions.m_mode = lodMesh.m_flags.tessellation
            ? backend::DrawOptions::Mode::patches
            : backend::DrawOptions::Mode::triangles;
    }

    backend::DrawOptions::Mode ModelMesh::getDrawMode()
    {
        return backend::DrawOptions::Mode::triangles;
    }
}
