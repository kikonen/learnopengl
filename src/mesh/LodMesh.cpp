#include "LodMesh.h"

#include <regex>

#include <fmt/format.h>

#include "util/Log.h"
#include "util/glm_util.h"

#include "asset/Assets.h"
#include "asset/Material.h"
#include "asset/Program.h"

#include "util/Util.h"

#include "kigl/GLVertexArray.h"

#include "render/size.h"

#include "registry/MaterialRegistry.h"

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
        m_meshIndex = o.m_meshIndex;
        m_socketIndex = o.m_socketIndex;

        m_material = std::move(o.m_material);
        m_materialIndex = std::move(o.m_materialIndex);

        m_lod = o.m_lod;
        m_deleter = std::move(o.m_deleter);
        m_vao = o.m_vao;

        m_program = o.m_program;
        m_shadowProgram = o.m_shadowProgram;
        m_preDepthProgram = o.m_preDepthProgram;
        m_selectionProgram = o.m_selectionProgram;
        m_idProgram = o.m_idProgram;
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
        m_meshIndex = o.m_meshIndex;
        m_socketIndex = o.m_socketIndex;

        m_material = std::move(o.m_material);
        m_materialIndex = std::move(o.m_materialIndex);

        m_lod = o.m_lod;
        m_deleter = std::move(o.m_deleter);
        m_vao = o.m_vao;

        o.m_program = o.m_program;
        o.m_shadowProgram = o.m_shadowProgram;
        o.m_preDepthProgram = o.m_preDepthProgram;
        o.m_selectionProgram = o.m_selectionProgram;
        o.m_idProgram = o.m_idProgram;
        o.m_drawOptions = o.m_drawOptions;

        m_flags = o.m_flags;

        o.m_mesh = nullptr;
        o.m_material = nullptr;

        return *this;
    }

    std::string LodMesh::str() const noexcept
    {
        return fmt::format(
            "<LOD_MESH: level={}, index={}, vao={}, mesh={}, material={}, socket={}>",
            m_levelMask,
            m_meshIndex,
            m_vao ? *m_vao : -1,
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

        m_program = material.getProgram(MaterialProgramType::shader);
        m_shadowProgram = material.getProgram(MaterialProgramType::shadow);
        m_preDepthProgram = material.getProgram(MaterialProgramType::pre_depth);
        m_selectionProgram = material.getProgram(MaterialProgramType::selection);
        m_idProgram = material.getProgram(MaterialProgramType::object_id);

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

        setMaterial(mesh->getMaterial());

        m_meshIndex = mesh->m_registeredIndex;
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

        MaterialRegistry::get().registerMaterial(*material);

        m_materialIndex = material->m_registeredIndex;

        // TODO KI basically material could be deleted at this point
    }

    void LodMesh::prepareRT(const PrepareContext& ctx)
    {
        if (m_mesh) {
            m_vao = m_mesh->prepareRT(ctx);
            m_mesh->prepareLodMesh(*this);

            if (m_flags.billboard) m_drawOptions.m_flags |= INSTANCE_BILLBOARD_BIT;
        }

        if (m_program) {
            m_program->prepareRT();
        }

        if (m_shadowProgram) {
            m_shadowProgram->prepareRT();
        }

        if (m_preDepthProgram) {
            m_preDepthProgram->prepareRT();
        }

        if (m_selectionProgram) {
            m_selectionProgram->prepareRT();
        }

        if (m_idProgram) {
            m_idProgram->prepareRT();
        }
    }
}
