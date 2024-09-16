#include "LodMesh.h"

#include <regex>

#include <fmt/format.h>

#include "util/Log.h"
#include "util/glm_util.h"

#include "asset/Assets.h"

#include "material/Material.h"

#include "shader/Program.h"

#include "util/Util.h"

#include "kigl/GLVertexArray.h"

#include "render/size.h"

#include "Mesh.h"
#include "InstanceFlags.h"

namespace {
    //const std::vector<std::regex> lodMatchers{
    //    std::regex(".*lod[0-9]+"),
    //};

    //int16_t resolveLodLevel(const std::string& name)
    //{
    //    const auto& str = util::toLower(name);
    //    if (!util::matchAny(lodMatchers, str)) return -1;

    //    std::string output = std::regex_replace(
    //        str,
    //        std::regex(".*lod([0-9]+)"),
    //        std::string("$1")
    //    );

    //    return stoi(output);
    //}
}

namespace mesh {
    LodMesh::LodMesh()
    {}

    LodMesh::LodMesh(Mesh* mesh)
    {
        setMesh(mesh);
    }

    LodMesh::LodMesh(LodMesh&& o) noexcept
    {
        m_levelMask = o.m_levelMask;
        m_priority = o.m_priority;

        m_mesh = o.m_mesh;
        m_socketIndex = o.m_socketIndex;

        m_scale = o.m_scale;
        m_baseScale = o.m_baseScale;
        m_baseRotation = o.m_baseRotation;

        m_animationRigTransform = o.m_animationRigTransform;
        m_transform = o.m_transform;

        m_material = std::move(o.m_material);
        m_materialIndex = std::move(o.m_materialIndex);

        m_deleter = std::move(o.m_deleter);
        m_vaoId = o.m_vaoId;

        m_baseVertex = o.m_baseVertex;
        m_baseIndex = o.m_baseIndex;
        m_indexCount = o.m_indexCount;

        m_programId = o.m_programId;
        m_shadowProgramId = o.m_shadowProgramId;
        m_preDepthProgramId = o.m_preDepthProgramId;
        m_selectionProgramId = o.m_selectionProgramId;
        m_idProgramId = o.m_idProgramId;
        m_drawOptions = o.m_drawOptions;

        m_flags = o.m_flags;

        o.m_mesh = nullptr;
        o.m_material = nullptr;
    }

    LodMesh::~LodMesh()
    {}

    LodMesh& LodMesh::operator=(LodMesh&& o) noexcept
    {
        m_levelMask = o.m_levelMask;
        m_priority = o.m_priority;

        m_mesh = o.m_mesh;
        m_socketIndex = o.m_socketIndex;

        m_scale = o.m_scale;
        m_baseScale = o.m_baseScale;

        m_material = std::move(o.m_material);
        m_materialIndex = std::move(o.m_materialIndex);

        m_deleter = std::move(o.m_deleter);
        m_vaoId = o.m_vaoId;

        m_baseVertex = o.m_baseVertex;
        m_baseIndex = o.m_baseIndex;
        m_indexCount = o.m_indexCount;

        m_programId = o.m_programId;
        m_shadowProgramId = o.m_shadowProgramId;
        m_preDepthProgramId = o.m_preDepthProgramId;
        m_selectionProgramId = o.m_selectionProgramId;
        m_idProgramId = o.m_idProgramId;
        m_drawOptions = o.m_drawOptions;

        m_animationRigTransform = o.m_animationRigTransform;

        m_flags = o.m_flags;

        o.m_mesh = nullptr;
        o.m_material = nullptr;

        return *this;
    }

    std::string LodMesh::str() const noexcept
    {
        return fmt::format(
            "<LOD_MESH: level={}, vao={}, mesh={}, material={}, socket={}>",
            m_levelMask,
            m_vaoId,
            m_mesh ? m_mesh->str() : "N/A",
            m_materialIndex,
            m_socketIndex);
    }

    Material* LodMesh::getMaterial() noexcept
    {
        return m_material.get();
    }

    void LodMesh::setMaterial(const Material& src) noexcept
    {
        // NOTE KI copy of material for isntance
        // => material *is* per mesh type
        // => Sharing *might* be sometims possible, in practice tricky
        if (!m_material) {
            m_material = std::make_unique<Material>();
        }
        *m_material = src;

        setupDrawOptions();
    }

    void LodMesh::setupDrawOptions()
    {
        if (!m_material) return;

        auto& material = *m_material;

        m_drawOptions.m_alpha = material.alpha;
        m_drawOptions.m_blend = material.blend;

        m_drawOptions.m_gbuffer = material.gbuffer;

        m_drawOptions.m_renderBack = material.renderBack;
        m_drawOptions.m_wireframe = material.wireframe;

        if (m_drawOptions.m_alpha) {
            m_drawOptions.m_kindBits |= render::KIND_ALPHA;
        }
        if (m_drawOptions.m_blend) {
            m_drawOptions.m_kindBits |= render::KIND_BLEND;
        }
        if (m_drawOptions.m_kindBits == 0) {
            m_drawOptions.m_kindBits = render::KIND_SOLID;
            m_drawOptions.m_solid = true;
        }

        m_programId = material.getProgram(MaterialProgramType::shader);
        m_shadowProgramId = material.getProgram(MaterialProgramType::shadow);
        m_preDepthProgramId = material.getProgram(MaterialProgramType::pre_depth);
        m_selectionProgramId = material.getProgram(MaterialProgramType::selection);
        m_idProgramId = material.getProgram(MaterialProgramType::object_id);

        if (m_flags.zUp) {
            const auto rotateYUp = util::degreesToQuat(glm::vec3{ 90.f, 0.f, 0.f });
            m_animationRigTransform = glm::toMat4(rotateYUp);
        }
    }

    void LodMesh::setMesh(
        std::unique_ptr<Mesh> mesh,
        bool umique) noexcept
    {
        setMesh(mesh.get());
        m_deleter = std::move(mesh);
    }

    void LodMesh::setMesh(Mesh* mesh) noexcept
    {
        m_mesh = mesh;
        if (!m_mesh) return;

        m_flags.noVolume = m_mesh->m_flags.noVolume;
        m_flags.useBones = m_mesh->m_flags.useBones;
        m_flags.boneVisualization = m_mesh->m_flags.boneVisualization;

        setMaterial(mesh->getMaterial());
    }

    void LodMesh::registerMaterial()
    {
        if (!m_mesh) return;

        const auto& assets = Assets::get();

        auto* material = m_material.get();

        if (!material) {
            throw "missing material";
        }

        if (assets.useLodDebug) {
            if (m_levelMask >= 1 << 0)
                material->kd = glm::vec4{ 0.5f, 0.f, 0.f, 1.f };
            if (m_levelMask >= 1 << 1)
                material->kd = glm::vec4{ 0.f, 0.5f, 0.5f, 1.f };
            if (m_levelMask >= 1 << 2)
                material->kd = glm::vec4{ 0.f, 0.f, 0.5f, 1.f };
            if (m_levelMask >= 1 << 3)
                material->kd = glm::vec4{ 0.f, 0.5f, 0.f, 1.f };
        }

        m_materialIndex = material->registerMaterial();

        // TODO KI basically material could be deleted at this point
    }

    void LodMesh::prepareRT(const PrepareContext& ctx)
    {
        if (m_mesh) {
            m_vaoId = static_cast<ki::vao_id>(*m_mesh->prepareVAO());
            m_mesh->prepareLodMesh(*this);

            updateTransform();

            if (m_flags.billboard) m_drawOptions.m_flags |= INSTANCE_BILLBOARD_BIT;
        }

        if (m_programId) {
            Program::get(m_programId)->prepareRT();
        }

        if (m_shadowProgramId) {
            Program::get(m_shadowProgramId)->prepareRT();
        }

        if (m_preDepthProgramId) {
            Program::get(m_preDepthProgramId)->prepareRT();
        }

        if (m_selectionProgramId) {
            Program::get(m_selectionProgramId)->prepareRT();
        }

        if (m_idProgramId) {
            Program::get(m_idProgramId)->prepareRT();
        }
    }

    void LodMesh::updateTransform() {
        // TODO KI rotate here causes very weird artifacts
        m_transform =
            glm::mat4(m_baseRotation) *
            glm::scale(glm::mat4{ 1.f }, m_scale * m_baseScale) *
            m_mesh->m_rigTransform;
    }

    AABB LodMesh::calculateAABB() const noexcept
    {
        if (!m_mesh) return {};

        AABB aabb{ true };

        if (m_mesh) {
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
