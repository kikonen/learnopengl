#include "MeshTypeBuilder.h"

#include <string>
#include <vector>
#include <fstream>
#include <string>
#include <span>

#include <fmt/format.h>

#include "util/util.h"
#include "util/file.h"
#include "util/glm_format.h"
#include "util/glm_util.h"

#include "ki/sid.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "asset/Assets.h"

#include "material/Material.h"

#include "shader/Program.h"
#include "shader/Shader.h"
#include "shader/ProgramRegistry.h"

#include "mesh/mesh_util.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/MeshSet.h"
#include "mesh/ModelMesh.h"
#include "mesh/TextMesh.h"
#include "mesh/NonVaoMesh.h"

#include "component/CameraDefinition.h"
#include "component/LightDefinition.h"

#include "generator/TextDefinition.h"

#include "particle/ParticleDefinition.h"

#include "model/NodeType.h"

#include "animation/AnimationLoader.h"
#include "animation/RigContainer.h"
#include "animation/RigSocket.h"

#include "mesh/MeshType.h"

#include "registry/Registry.h"
#include "registry/ModelRegistry.h"
#include "registry/MeshTypeRegistry.h"

#include "Loaders.h"

#include "loader_util.h"

namespace {
    const std::string QUAD_MESH_NAME{ "quad" };

    const std::string ANY_MATERIAL{ "*" };
}

namespace loader
{
    MeshTypeBuilder::MeshTypeBuilder(std::shared_ptr<Loaders> loaders)
        : m_loaders{ loaders }
    { }

    MeshTypeBuilder::~MeshTypeBuilder() = default;

    pool::TypeHandle MeshTypeBuilder::createType(
        const NodeData& nodeData,
        const std::string& nameSuffix)
    {
        auto& l = *m_loaders;

        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();

        std::string name = nodeData.baseId.m_path.empty() ? nodeData.name : nodeData.baseId.m_path;
        if (!nameSuffix.empty()) {
            name = fmt::format("{}_{}", name, nameSuffix);
        }

        type->setName(name);
        type->m_layer = nodeData.layer;

        assignTypeFlags(nodeData, type->m_flags);

        if (nodeData.type == NodeType::origo) {
            type->m_flags.origo = true;
            type->m_flags.invisible = true;
        }
        else
        {
            resolveMeshes(type, nodeData);

            // NOTE KI container does not have mesh itself, but it can setup
            // material & program for contained nodes
            if (nodeData.type != NodeType::container) {
                if (!type->hasMesh()) {
                    KI_WARN(fmt::format(
                        "SCENE_FILEIGNORE: NO_MESH id={} ({})",
                        nodeData.baseId, nodeData.desc));
                    return pool::TypeHandle::NULL_HANDLE;
                }
            }

            int deferredCount = 0;
            int forwardCount = 0;
            int oitCount = 0;
            for (const auto& lodMesh : type->getLodMeshes()) {
                if (lodMesh.m_material->gbuffer) {
                    deferredCount++;
                    if (lodMesh.m_drawOptions.isBlend()) {
                        oitCount++;
                    }
                }
                else {
                    forwardCount++;
                }
            }
            type->m_flags.useDeferred |= deferredCount > 0;
            type->m_flags.useForward |= forwardCount > 0;
            type->m_flags.useOit |= oitCount > 0;
        }

        {
            type->setCustomMaterial(
                m_loaders->m_customMaterialLoader.createCustomMaterial(
                    nodeData.customMaterial,
                    *m_loaders));
        }

        {
            const auto& scriptIds = m_loaders->m_scriptLoader.createScripts(
                nodeData.scripts);

            for (auto& scriptId : scriptIds) {
                type->addScript(scriptId);
            }

            m_loaders->m_scriptLoader.bindTypeScripts(
                type->toHandle(),
                scriptIds);
        }

        resolveAttachments(type, nodeData);

        type->m_cameraDefinition = l.m_cameraLoader.createDefinition(nodeData.camera);
        type->m_lightDefinition = l.m_lightLoader.createDefinition(nodeData.light);
        type->m_particleDefinition = l.m_particleLoader.createDefinition(nodeData.particle);

        if (type->m_flags.text) {
            type->m_textDefinition = m_loaders->m_textLoader.createDefinition(
                type,
                nodeData.text,
                *m_loaders);
        }

        return typeHandle;
    }

    void MeshTypeBuilder::resolveMaterials(
        mesh::MeshType* type,
        mesh::LodMesh& lodMesh,
        const NodeData& nodeData,
        const MeshData& meshData,
        const LodData* lodData)
    {
        auto* lodMaterial = lodMesh.modifyMaterial();
        if (!lodMaterial) return;

        auto& material = *lodMaterial;

        if (!material.inmutable)
        {
            auto& l = *m_loaders;

            // ASSIGN - ANY
            for (auto& materialData : meshData.materials) {
                const auto& alias = materialData.aliasName;
                const auto& name = materialData.materialName;

                if (alias == ANY_MATERIAL)
                {
                    KI_INFO_OUT(fmt::format(
                        "MAT_ASSIGN: model={}, mesh={}, material={}, name={}, alias={}",
                        type->getName(),
                        lodMesh.getMeshName(),
                        material.m_name,
                        name,
                        alias));

                    material.assign(materialData.material);
                }
            }

            // ASSIGN - specific
            for (auto& materialData : meshData.materials) {
                const auto& alias = materialData.aliasName;
                const auto& name = materialData.materialName;

                if (name == material.m_name || alias == material.m_name)
                {
                    KI_INFO_OUT(fmt::format(
                        "MAT_ASSIGN: model={}, mesh={}, material={}, name={}, alias={}",
                        type->getName(),
                        lodMesh.getMeshName(),
                        material.m_name,
                        name,
                        alias));

                    material.assign(materialData.material);
                }
            }

            // NOTE KI settings from mesh to material *AFTER* assign, *BEFORE* modify
            if (meshData.defaultPrograms) {
                material.m_defaultPrograms = true;
            }

            {
                for (const auto& srcIt : nodeData.programs) {
                    const auto& dstIt = material.m_programNames.find(srcIt.first);
                    if (dstIt == material.m_programNames.end()) {
                        material.m_programNames[srcIt.first] = srcIt.second;
                    }
                }

                for (const auto& srcIt : meshData.programs) {
                    const auto& dstIt = material.m_programNames.find(srcIt.first);
                    if (dstIt == material.m_programNames.end()) {
                        material.m_programNames[srcIt.first] = srcIt.second;
                    }
                }
            }

            // MODIFY - BASE - ANY
            for (auto& materialData : meshData.materialModifiers) {
                const auto& alias = materialData.aliasName;
                const auto& name = materialData.materialName;

                if (alias == ANY_MATERIAL)
                {
                    KI_INFO_OUT(fmt::format(
                        "MAT_MODIFY: model={}, mesh={}, material={}, name={}, alias={}",
                        type->getName(),
                        lodMesh.getMeshName(),
                        material.m_name,
                        name,
                        alias));

                    l.m_materialLoader.modifyMaterial(material, materialData);
                }
            }

            // MODIFY - specific material
            for (auto& materialData : meshData.materialModifiers) {
                const auto& alias = materialData.aliasName;
                const auto& name = materialData.materialName;

                if (name == material.m_name || alias == material.m_name)
                {
                    KI_INFO_OUT(fmt::format(
                        "MAT_MODIFY: model={}, mesh={}, material={}, name={}, alias={}",
                        type->getName(),
                        lodMesh.getMeshName(),
                        material.m_name,
                        name,
                        alias));

                    l.m_materialLoader.modifyMaterial(material, materialData);
                }
            }

            // MODIFY - LOD
            if (lodData) {
                // MODIFY LOD - ANY
                for (auto& materialData : lodData->materialModifiers) {
                    const auto& alias = materialData.aliasName;
                    const auto& name = materialData.materialName;

                    if (alias == "*")
                    {
                        KI_INFO_OUT(fmt::format(
                            "MAT_MODIFY: model={}, mesh={}, material={}, name={}, alias={}",
                            type->getName(),
                            lodMesh.getMeshName(),
                            material.m_name,
                            name,
                            alias));

                        l.m_materialLoader.modifyMaterial(material, materialData);
                    }
                }


                // MODIFY LOD - specific material
                for (auto& materialData : lodData->materialModifiers) {
                    const auto& alias = materialData.aliasName;
                    const auto& name = materialData.materialName;

                    if (name == material.m_name || alias == material.m_name)
                    {
                        KI_INFO_OUT(fmt::format(
                            "MAT_MODIFY: model={}, mesh={}, material={}, name={}, alias={}",
                            type->getName(),
                            lodMesh.getMeshName(),
                            material.m_name,
                            name,
                            alias));

                        l.m_materialLoader.modifyMaterial(material, materialData);
                    }
                }
            }
        }

        m_loaders->m_materialLoader.resolveMaterial(lodMesh.m_flags, material);
    }

    void MeshTypeBuilder::resolveMeshes(
        mesh::MeshType* type,
        const NodeData& nodeData)
    {
        uint16_t index = 0;
        for (const auto& meshData : nodeData.meshes) {
            if (!meshData.enabled) continue;
            resolveMesh(type, nodeData, meshData, index);
            index++;
        }

        // NOTE KI ensure volume is containing all meshes
        type->prepareVolume();
    }

    void MeshTypeBuilder::resolveMesh(
        mesh::MeshType* type,
        const NodeData& nodeData,
        const MeshData& meshData,
        int index)
    {
        const auto& assets = Assets::get();

        size_t startIndex = type->getLodMeshes().size();
        size_t meshCount = 0;

        // NOTE KI materials MUST be resolved before loading mesh
        if (nodeData.type == NodeType::model) {
            type->m_flags.model = true;
            meshCount += resolveModelMesh(type, nodeData, meshData, index);

            KI_INFO(fmt::format(
                "SCENE_FILE MESH: id={}, desc={}, type={}",
                nodeData.baseId, nodeData.desc, type->str()));
        }
        else if (nodeData.type == NodeType::text) {
            type->m_flags.text = true;
            auto mesh = std::make_shared<mesh::TextMesh>();

            if (!mesh->getMaterial()) {
                const auto& material = Material::createMaterial(BasicMaterial::yellow);
                mesh->setMaterial(&material);
            }

            mesh::LodMesh lodMesh;
            lodMesh.setMesh(mesh);
            type->addLodMesh(std::move(lodMesh));
            meshCount++;
        }
        else if (nodeData.type == NodeType::terrain) {
            // NOTE KI handled via container + generator
            type->m_flags.terrain = true;
            throw std::runtime_error("Terrain not supported currently");
        }
        else if (nodeData.type == NodeType::container) {
            // NOTE KI generator takes care of actual work
            type->m_flags.container = true;
            type->m_flags.invisible = true;
        }
        else {
            // NOTE KI root/origo/unknown; don't render, just keep it in hierarchy
            type->m_flags.origo = true;
            type->m_flags.invisible = true;
        }

        // Resolve materials for newly added meshes
        if (meshCount > 0) {
            const auto& span = std::span{ type->modifyLodMeshes() }.subspan(startIndex, meshCount);
            for (auto& lodMesh : span) {
                resolveLodMesh(type, nodeData, meshData, lodMesh);

                auto* mesh = lodMesh.getMesh<mesh::Mesh>();
                const auto rig = mesh ? mesh->getRigContainer() : nullptr;
                if (rig) {
                    rig->dump();
                }
            }
        }
    }

    int MeshTypeBuilder::resolveModelMesh(
        mesh::MeshType* type,
        const NodeData& nodeData,
        const MeshData& meshData,
        int index)
    {
        const auto& assets = Assets::get();
        int meshCount = 0;

        switch (meshData.type) {
        case MeshDataType::mesh: {
            auto future = ModelRegistry::get().getMeshSet(
                meshData.id,
                assets.modelsDir,
                meshData.path,
                meshData.smoothNormals,
                meshData.forceNormals);

            const auto& meshSet = future.get();

            if (meshSet) {
                resolveSockets(
                    meshData,
                    *meshSet
                );

                for (auto& animationData : meshData.animations) {
                    loadAnimation(
                        meshData.baseDir,
                        animationData,
                        *meshSet);
                }

                {
                    KI_INFO_OUT(fmt::format(
                        "\n=======================\n[MESH_SET SUMMARY: {}]\n{}\n=======================",
                        meshSet->m_name,
                        meshSet->getSummary()));
                }

                meshCount += type->addMeshSet(*meshSet);
            }
            break;
        }
        case MeshDataType::primitive: {
            auto mesh = m_loaders->m_vertexLoader.createMesh(meshData, meshData.vertexData, *m_loaders);

            mesh::LodMesh lodMesh;
            lodMesh.setMesh(mesh);
            type->addLodMesh(std::move(lodMesh));

            meshCount++;
            break;
        }
        case MeshDataType::non_vao: {
            auto mesh = std::make_shared<mesh::NonVaoMesh>(type->getName());

            if (meshData.materials.empty()) {
                const auto& material = Material::createMaterial(BasicMaterial::yellow);
                mesh->setMaterial(&material);
            }
            else {
                const auto* material = &meshData.materials[0].material;
                mesh->setMaterial(material);
            }

            mesh::LodMesh lodMesh;
            lodMesh.setMesh(mesh);
            type->addLodMesh(std::move(lodMesh));

            meshCount++;
            break;
        }
        }

        return meshCount;
    }

    void MeshTypeBuilder::resolveLodMesh(
        mesh::MeshType* type,
        const NodeData& nodeData,
        const MeshData& meshData,
        mesh::LodMesh& lodMesh)
    {
        lodMesh.m_scale = meshData.scale;
        lodMesh.m_baseScale = meshData.baseScale;
        lodMesh.m_baseRotation = util::degreesToQuat(meshData.baseRotation);

        auto* lodData = resolveLod(type, nodeData, meshData, lodMesh);

        assignMeshFlags(meshData.meshFlags, lodMesh.m_flags);
        if (lodData) {
            assignMeshFlags(lodData->meshFlags, lodMesh.m_flags);
        }

        resolveMaterials(type, lodMesh, nodeData, meshData, lodData);
        lodMesh.setupDrawOptions();
    }

    const LodData* MeshTypeBuilder::resolveLod(
        mesh::MeshType* type,
        const NodeData& nodeData,
        const MeshData& meshData,
        mesh::LodMesh& lodMesh)
    {
        auto* mesh = lodMesh.getMesh<mesh::Mesh>();
        if (!mesh) return nullptr;

        lodMesh.m_priority = nodeData.priority;

        const auto* lod = meshData.findLod(mesh->m_name);
        if (!lod) {
            // NOTE KI if lods defined then default to 0 mask
            if (!meshData.lods.empty()) {
                KI_WARN_OUT(fmt::format("SCENE: MISSING_LOD : mesh={}", mesh->m_name));
                //lodMesh.m_levelMask = 0;
            }
            return nullptr;
        }

        lodMesh.m_minDistance2 = lod->minDistance * lod->minDistance;
        lodMesh.m_maxDistance2 = lod->maxDistance * lod->maxDistance;
        lodMesh.m_priority = lod->priority != 0 ? lod->priority : nodeData.priority;

        return lod;
    }

    void MeshTypeBuilder::resolveSockets(
        const MeshData& meshData,
        mesh::MeshSet& meshSet)
    {
        if (!meshSet.m_rig) return;

        auto& rig = *meshSet.m_rig;
        for (const auto& socketData : meshData.sockets) {
            if (!socketData.enabled) continue;

            // TODO KI scale is in LodMesh level, but sockets in Mesh level
            // => PROBLEM if same mesh is used for differently scaled LodMeshes
            //glm::vec3 meshScale{ 0.01375f * 2.f };
            auto meshScale = meshData.scale * meshData.baseScale;

            animation::RigSocket socket{
                socketData.name,
                socketData.joint,
                socketData.offset,
                util::degreesToQuat(socketData.rotation),
                socketData.scale,
                meshScale
            };

            rig.registerSocket(socket);
        }

        rig.prepare();
    }

    void MeshTypeBuilder::loadAnimation(
        const std::string& baseDir,
        const AnimationData& data,
        mesh::MeshSet& meshSet)
    {
        if (!meshSet.isRigged()) return;

        const auto& assets = Assets::get();

        animation::AnimationLoader loader{};

        std::string filePath;
        {
            filePath = util::joinPathExt(
                meshSet.m_rootDir,
                baseDir,
                data.path, "");
        }

        if (!util::fileExists(filePath)) {
            filePath = util::joinPath(
                meshSet.m_rootDir,
                data.path);
        }

        try {
            loader.loadAnimations(
                *meshSet.m_rig,
                data.name,
                filePath);
        }
        catch (animation::AnimationNotFoundError ex) {
            KI_CRITICAL(fmt::format("SCENE_ERROR: LOAD_ANIMATION - {}", ex.what()));
            //throw std::current_exception();
        }
    }

    void MeshTypeBuilder::resolveAttachments(
        mesh::MeshType* type,
        const NodeData& nodeData)
    {
        if (nodeData.attachments.empty()) return;

        auto& lodMeshes = type->modifyLodMeshes();

        auto rig = mesh::findRig(lodMeshes);
        if (!rig) {
            KI_INFO_OUT(fmt::format(
                "SOCKET_BIND_ERROR: rig_missing - node={}",
                nodeData.str()));
            return;
        }

        for (const auto& attachment : nodeData.attachments) {
            if (!attachment.enabled) continue;

            mesh::LodMesh* lodMesh = mesh::findLodMesh(attachment.name, lodMeshes);
            if (!lodMesh) {
                const auto& names = mesh::getLodMeshNames(attachment.name, lodMeshes);
                const auto& aliases = mesh::getLodMeshNames(attachment.name, lodMeshes);
                KI_INFO_OUT(fmt::format(
                    "SOCKET_BIND_ERROR: mesh_missing - node={}, rig={}, socket={}, mesh={}, mesh_names=[{}], mesh_aliases=[{}]",
                    nodeData.str(),
                    rig->m_name,
                    attachment.socket,
                    attachment.name,
                    util::join(names, ", "),
                    util::join(aliases, ", ")));
                continue;
            }

            const auto* socket = rig->findSocket(attachment.socket);
            if (!socket) {
                const auto& names = rig->getSocketNames();
                KI_INFO_OUT(fmt::format(
                    "SOCKET_BIND_ERROR: socket_missing - node={}, rig={}, socket={}, mesh={}, socket_names=[{}]",
                    nodeData.str(),
                    rig->m_name,
                    attachment.socket,
                    attachment.name,
                    util::join(names, ", ")));
                continue;
            }

            lodMesh->m_socketIndex = socket->m_index;

            KI_INFO_OUT(fmt::format(
                "SOCKET_BIND_OK: node={}, rig={}, joint={}.{}, socket={}.{}, mesh={}",
                nodeData.str(),
                rig->m_name,
                socket->m_jointIndex,
                socket->m_jointName,
                socket->m_index,
                attachment.socket,
                attachment.name));
        }
    }

    void MeshTypeBuilder::assignTypeFlags(
        const NodeData& nodeData,
        mesh::TypeFlags& flags)
    {
        const auto& container = nodeData.typeFlags;

        //////////////////////////////////////////////////////////
        // LOD_MESH specific
        // Rigged model

        flags.effect = container.getFlag("effect", flags.effect);

        //////////////////////////////////////////////////////////
        // MESH_TYPE specific (aka. Node shared logic)

        flags.mirror = container.getFlag("mirror", flags.mirror);
        flags.water = container.getFlag("water", flags.water);
        flags.cubeMap = container.getFlag("cube_map", flags.cubeMap);

        flags.useDeferred = container.getFlag("use_deferred", flags.useDeferred);
        flags.useForward = container.getFlag("use_forward", flags.useForward);
        flags.useOit = container.getFlag("use_oit", flags.useOit);

        flags.noFrustum = container.getFlag("no_frustum", flags.noFrustum);
        flags.noShadow = container.getFlag("no_shadow", flags.noShadow);
        flags.noSelect = container.getFlag("no_select", flags.noSelect);
        flags.noReflect = container.getFlag("no_reflect", flags.noReflect);
        flags.noRefract = container.getFlag("no_refract", flags.noRefract);

        {
            flags.staticBounds = container.getFlag("static_bounds", flags.staticBounds);
            flags.dynamicBounds = container.getFlag("dynamic_bounds", flags.dynamicBounds);
            flags.physics = nodeData.physics.enabled;
        }

        flags.navMesh = container.getFlag("nav_mesh", flags.navMesh);
        flags.navPhysics = container.getFlag("nav_physics", flags.navPhysics);
    }

    void MeshTypeBuilder::assignMeshFlags(
        const FlagContainer& container,
        mesh::MeshFlags& flags)
    {
        flags.billboard = container.getFlag("billboard", flags.billboard);
        flags.tessellation = container.getFlag("tessellation", flags.tessellation);

        flags.preDepth = container.getFlag("pre_depth", flags.preDepth);

        flags.noVolume = container.getFlag("no_volume", flags.noVolume);

        {
            flags.useBones = container.getFlag("use_bones", flags.useBones);

            // NOTE KI bones are *required* if using animation
            flags.useAnimation = container.getFlag("use_animation", flags.useAnimation);
            if (flags.useAnimation) {
                flags.useBones = true;
            }

            // NOTE KI no bones debug if no bones
            flags.useBonesDebug = container.getFlag("use_bones_debug", flags.useBonesDebug);
            if (!flags.useBones) {
                flags.useBonesDebug = false;
            }

            flags.useSockets = container.getFlag("use_sockets", flags.useSockets);
        }

        flags.clip = container.getFlag("clip", flags.clip);
    }
}
