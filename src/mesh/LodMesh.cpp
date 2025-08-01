#include "LodMesh.h"

#include <regex>

#include <fmt/format.h>

#include "util/Log.h"
#include "util/glm_util.h"

#include "asset/Assets.h"

#include "material/Material.h"

#include "shader/Program.h"

#include "util/util.h"

#include "kigl/GLVertexArray.h"

#include "render/size.h"

#include "Mesh.h"
#include "InstanceFlags.h"

namespace {
}

namespace mesh {
    LodMesh::LodMesh()
    {}

    LodMesh::LodMesh(std::shared_ptr<Mesh> mesh)
    {
        setMesh(mesh);
    }

    LodMesh::LodMesh(LodMesh&& o) noexcept
    {
        m_priority = o.m_priority;

        m_mesh = o.m_mesh;
        m_socketIndex = o.m_socketIndex;

        m_scale = o.m_scale;
        m_baseScale = o.m_baseScale;
        m_baseRotation = o.m_baseRotation;

        //m_animationRigTransform = o.m_animationRigTransform;
        m_baseTransform = o.m_baseTransform;

        m_material = std::move(o.m_material);
        m_materialIndex = o.m_materialIndex;

        m_vaoId = o.m_vaoId;

        m_minDistance2 = o.m_minDistance2;
        m_maxDistance2 = o.m_maxDistance2;

        m_baseVertex = o.m_baseVertex;
        m_baseIndex = o.m_baseIndex;
        m_indexCount = o.m_indexCount;

        m_programId = o.m_programId;
        m_oitProgramId = o.m_oitProgramId;
        m_shadowProgramId = o.m_shadowProgramId;
        m_preDepthProgramId = o.m_preDepthProgramId;
        m_selectionProgramId = o.m_selectionProgramId;
        m_idProgramId = o.m_idProgramId;
        m_normalProgramId = o.m_normalProgramId;
        m_drawOptions = o.m_drawOptions;

        m_flags = o.m_flags;

        o.m_material = nullptr;
    }

    LodMesh::~LodMesh()
    {}

    LodMesh& LodMesh::operator=(LodMesh&& o) noexcept
    {
        m_priority = o.m_priority;

        m_mesh = o.m_mesh;
        m_socketIndex = o.m_socketIndex;

        m_scale = o.m_scale;
        m_baseScale = o.m_baseScale;

        m_material = std::move(o.m_material);
        m_materialIndex = o.m_materialIndex;

        m_vaoId = o.m_vaoId;

        m_minDistance2 = o.m_minDistance2;
        m_maxDistance2 = o.m_maxDistance2;

        m_baseVertex = o.m_baseVertex;
        m_baseIndex = o.m_baseIndex;
        m_indexCount = o.m_indexCount;

        m_programId = o.m_programId;
        m_oitProgramId = o.m_oitProgramId;
        m_shadowProgramId = o.m_shadowProgramId;
        m_preDepthProgramId = o.m_preDepthProgramId;
        m_selectionProgramId = o.m_selectionProgramId;
        m_idProgramId = o.m_idProgramId;
        m_normalProgramId = o.m_normalProgramId;
        m_drawOptions = o.m_drawOptions;

        //m_animationRigTransform = o.m_animationRigTransform;

        m_flags = o.m_flags;

        o.m_material = nullptr;

        return *this;
    }

    std::string LodMesh::str() const noexcept
    {
        return fmt::format(
            "<LOD_MESH: min={}, max={}, vao={}, mesh={}, material={}, socket={}>",
            sqrt(m_minDistance2),
            sqrt(m_maxDistance2),
            m_vaoId,
            m_mesh ? m_mesh->str() : "N/A",
            m_materialIndex,
            m_socketIndex);
    }

    const Material* LodMesh::getMaterial() const noexcept
    {
        return m_material.get();
    }

    Material* LodMesh::modifyMaterial() noexcept
    {
        return m_material.get();
    }

    void LodMesh::setMaterial(const Material* src) noexcept
    {
        if (!src) {
            m_material.reset();
            return;
        }

        // NOTE KI copy of material for isntance
        // => material *is* per mesh type
        // => Sharing *might* be sometims possible, in practice tricky
        if (!m_material) {
            m_material = std::make_unique<Material>();
        }
        *m_material = *src;

        m_materialIndex = m_material->m_registeredIndex;

        setupDrawOptions();
    }

    void LodMesh::clearMaterial() noexcept
    {
        m_material.reset();
    }

    void LodMesh::setupDrawOptions()
    {
        if (!m_material) return;

        auto& material = *m_material;

        if (material.alpha) {
            m_drawOptions.m_kindBits |= render::KIND_ALPHA;
        }
        if (material.blend) {
            m_drawOptions.m_kindBits |= render::KIND_ALPHA;
            m_drawOptions.m_kindBits |= render::KIND_BLEND;
        }
        if (m_drawOptions.m_kindBits == 0) {
            m_drawOptions.m_kindBits = render::KIND_SOLID;
        }

        m_drawOptions.m_gbuffer = material.gbuffer;

        m_drawOptions.m_renderBack = material.renderBack;
        m_drawOptions.m_lineMode = material.lineMode;
        m_drawOptions.m_reverseFrontFace = material.reverseFrontFace;
        m_drawOptions.m_noDepth = material.noDepth;

        m_drawOptions.m_clip = m_flags.clip;

        if (m_flags.billboard) m_drawOptions.m_flags |= INSTANCE_BILLBOARD_BIT;

        m_programId = material.getProgram(MaterialProgramType::shader);
        m_oitProgramId = material.getProgram(MaterialProgramType::oit);
        m_shadowProgramId = material.getProgram(MaterialProgramType::shadow);
        m_preDepthProgramId = material.getProgram(MaterialProgramType::pre_depth);
        m_selectionProgramId = material.getProgram(MaterialProgramType::selection);
        m_idProgramId = material.getProgram(MaterialProgramType::object_id);
        m_normalProgramId = material.getProgram(MaterialProgramType::normal);
    }

    std::string LodMesh::getMeshName()
    {
        auto* mesh = getMesh<mesh::Mesh>();
        return mesh ? mesh->m_name : "NA";
    }

    void LodMesh::setMesh(
        std::shared_ptr<Mesh> mesh) noexcept
    {
        m_mesh = mesh;
        if (!m_mesh) return;

        if (m_mesh->isJointVisualization()) {
            m_flags.noVolume = true;
            m_flags.useBones = true;
            m_flags.boneVisualization = true;
        }

        setMaterial(mesh->getMaterial());

        const auto& assets = Assets::get();

        m_minDistance2 = assets.nearPlane;
        m_minDistance2 *= m_minDistance2;

        m_maxDistance2 = assets.farPlane;
        m_maxDistance2 *= m_maxDistance2;
    }

    void LodMesh::registerMaterial()
    {
        if (!m_mesh) return;

        const auto& assets = Assets::get();

        auto* material = m_material.get();

        if (!material) {
            throw "missing material";
        }

        //if (assets.useLodDebug) {
        //    if (m_levelMask >= 1 << 0)
        //        material->kd = glm::vec4{ 0.5f, 0.f, 0.f, 1.f };
        //    if (m_levelMask >= 1 << 1)
        //        material->kd = glm::vec4{ 0.f, 0.5f, 0.5f, 1.f };
        //    if (m_levelMask >= 1 << 2)
        //        material->kd = glm::vec4{ 0.f, 0.f, 0.5f, 1.f };
        //    if (m_levelMask >= 1 << 3)
        //        material->kd = glm::vec4{ 0.f, 0.5f, 0.f, 1.f };
        //}

        m_materialIndex = material->registerMaterial();

        // TODO KI basically material could be deleted at this point
    }

    void LodMesh::prepareRT(const PrepareContext& ctx)
    {
        if (m_mesh) {
            if (auto* vao = m_mesh->prepareVAO(); vao) {
                m_vaoId = static_cast<ki::vao_id>(*vao);
            }
            m_mesh->prepareLodMesh(*this);

            updateTransform();
        }
    }

    void LodMesh::updateTransform() {
        // TODO KI rotate here causes very weird artifacts
        m_baseTransform =
            glm::mat4(m_baseRotation) *
            glm::scale(glm::mat4{ 1.f }, m_scale * m_baseScale) *
            m_mesh->m_rigTransform;
    }

    AABB LodMesh::calculateAABB() const noexcept
    {
        if (!m_mesh) return {};

        AABB aabb{ true };

        {
            // TODO KI rotate here causes very weird artifacts
            const auto& transform =
                glm::mat4(m_baseRotation) *
                glm::scale(glm::mat4{ 1.f }, m_scale * m_baseScale) *
                m_mesh->m_rigTransform;
            aabb.minmax(m_mesh->calculateAABB(transform));
        }

        aabb.updateVolume();

        return aabb;
    }
}
