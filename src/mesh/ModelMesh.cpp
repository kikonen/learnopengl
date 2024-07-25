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
        : Mesh{ name }
    {
    }

    ModelMesh::~ModelMesh()
    {
        KI_INFO(fmt::format("MODEL_MESH: delete - {}", str()));
        m_vertices.clear();
    }

    const kigl::GLVertexArray* ModelMesh::prepareVAO()
    {
        if (m_preparedVAO) return m_vao;
        m_preparedVAO = true;

        TexturedVAO* vao;

        if (m_rig && !m_vertexBones.empty()) {
            vao = VaoRegistry::get().getSkinnedVao();
        }
        else {
            vao = VaoRegistry::get().getTexturedVao();
        }

        return setupVAO(vao);
    }

    const kigl::GLVertexArray* ModelMesh::setupVAO(mesh::TexturedVAO* vao)
    {
        m_vboIndex = vao->reserveVertices(m_vertices.size());
        m_eboIndex = vao->reserveIndeces(m_indeces.size());

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

        m_vao = vao->getVAO();
        return m_vao;
    }

    void ModelMesh::prepareLodMesh(
        mesh::LodMesh& lodMesh)
    {
        auto& lod = lodMesh.m_lod;

        lod.m_baseVertex = getBaseVertex();
        lod.m_baseIndex = getBaseIndex();
        lod.m_indexCount = getIndexCount();

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
