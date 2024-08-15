#include "PrimitiveMesh.h"

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
    PrimitiveMesh::PrimitiveMesh()
        : Mesh("primitive"),
        m_type{ PrimitiveType::none }
    {
    }

    PrimitiveMesh::PrimitiveMesh(std::string_view name)
        : Mesh(name),
        m_type{ PrimitiveType::none }
    {
    }

    PrimitiveMesh::~PrimitiveMesh()
    {
    }

    void PrimitiveMesh::clear() {
        m_vertices.clear();
        m_indeces.clear();
    }

    const kigl::GLVertexArray* PrimitiveMesh::prepareVAO()
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

    const kigl::GLVertexArray* PrimitiveMesh::setupVAO(mesh::TexturedVAO* vao, bool shared)
    {
        if (shared) {
            if (m_preparedVAO) return m_vao;
            m_preparedVAO = true;
        }

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

    void PrimitiveMesh::prepareLodMesh(
        mesh::LodMesh& lodMesh)
    {
        auto& lod = lodMesh.m_lod;

        lod.m_baseVertex = getBaseVertex();
        lod.m_baseIndex = getBaseIndex();
        lod.m_indexCount = getIndexCount();

        auto& drawOptions = lodMesh.m_drawOptions;
        drawOptions.m_type = backend::DrawOptions::Type::elements;
        drawOptions.m_mode = getDrawMode();

    }

    backend::DrawOptions::Mode PrimitiveMesh::getDrawMode()
    {
        switch (m_type) {
        case PrimitiveType::lines:
            return backend::DrawOptions::Mode::lines;
        case PrimitiveType::points:
            return backend::DrawOptions::Mode::points;
        case PrimitiveType::ray:
            return backend::DrawOptions::Mode::lines;
        }
        return backend::DrawOptions::Mode::triangles;
    }
}
