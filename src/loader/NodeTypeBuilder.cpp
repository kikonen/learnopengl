#include "NodeTypeBuilder.h"

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
#include "mesh/MeshSet.h"
#include "mesh/ModelMesh.h"
#include "mesh/TextMesh.h"
#include "mesh/NonVaoMesh.h"

#include "model/NodeType.h"
#include "model/CompositeDefinition.h"

#include "component/definition/CameraComponentDefinition.h"
#include "component/definition/LightDefinition.h"
#include "component/definition/AudioListenerDefinition.h"
#include "component/definition/AudioSourceDefinition.h"
#include "component/definition/PhysicsDefinition.h"
#include "component/definition/TextGeneratorDefinition.h"
#include "component/definition/ParticleGeneratorDefinition.h"

#include "animation/RigContainer.h"
#include "animation/Rig.h"
#include "animation/RigSocket.h"
#include "animation/Clip.h"

#include "mesh_set/AnimationImporter.h"

#include "registry/Registry.h"
#include "registry/MeshSetRegistry.h"
#include "registry/NodeTypeRegistry.h"

#include "Loaders.h"

#include "loader_util.h"

#include "CompositeData.h"
#include "ParticleData.h"

namespace {
    inline const std::string QUAD_MESH_NAME{ "quad" };

    //void resolveNodeTypeData(
    //    loader::NodeTypeData& typeData,
    //    const loader::NodeTypeData& src,
    //    const std::vector<loader::NodeTypeData>& nodeTypes)
    //{
    //    if (!src.parentId.empty()) {
    //        const auto* parentType = findNodeTypeData(src.parentId, nodeTypes);
    //        if (!parentType) {
    //            throw fmt::format("NODE_TYPE: parent_missing: {}", src.parentId.str());
    //        }
    //        resolveNodeTypeData(typeData, *parentType, nodeTypes);
    //    }
    //    typeData.merge(src);
    //}

    const loader::CompositeData* findComposite(
        loader::BaseId compositeId,
        const std::vector<loader::CompositeData>& composites)
    {
        const auto& it = std::find_if(
            composites.cbegin(),
            composites.cend(),
            [&compositeId](const auto& e) { return e.baseId == compositeId; });
        return it != composites.end() ? &(*it) : nullptr;
    }

    const loader::ParticleData* findParticle(
        loader::BaseId particleId,
        const std::vector<loader::ParticleData>& particles)
    {
        const auto& it = std::find_if(
            particles.cbegin(),
            particles.cend(),
            [&particleId](const auto& e) { return e.baseId == particleId; });
        return it != particles.end() ? &(*it) : nullptr;
    }
}

namespace loader
{
    NodeTypeBuilder::NodeTypeBuilder(
        const std::shared_ptr<Context>& ctx,
        const std::shared_ptr<Loaders>& loaders)
        : m_ctx{ ctx },
        m_loaders{ loaders }
    { }

    NodeTypeBuilder::~NodeTypeBuilder() = default;

    void NodeTypeBuilder::createTypes(
        const std::vector<NodeTypeData>& types,
        const std::vector<CompositeData>& composites,
        const std::vector<ParticleData>& particles)
    {
        for (const auto& typeData : types) {
            //NodeTypeData resolvedData;
            //resolveNodeTypeData(resolvedData, typeData, types);
            //createType(resolvedData);
            createType(typeData, composites, particles);
        }
    }

    pool::TypeHandle NodeTypeBuilder::createType(
        const NodeTypeData& typeData,
        const std::vector<CompositeData>& composites,
        const std::vector<ParticleData>& particles)
    {
        auto& l = *m_loaders;

        std::string typeName = typeData.baseId.getId();

        if (typeName.empty()) {
            throw fmt::format("type_id missing: {}", typeData.str());
        }

        auto typeHandle = pool::TypeHandle::allocate(SID(typeName));
        auto* type = typeHandle.toType();

        NodeTypeRegistry::get().registerType(typeHandle);

        type->setName(typeName);
        type->m_layer = typeData.layer;

        type->m_pivotPoint = typeData.pivot;
        type->m_front = typeData.front;
        type->m_baseScale = typeData.baseScale;
        type->m_baseRotation =util::degreesToQuat(typeData.baseRotation);

        assignTypeFlags(typeData, type->m_flags);

        if (!typeData.enabled) {
            KI_INFO_OUT(fmt::format(
                "TYPE::NOT_ENABLED: type={}",
                typeName));
            type->m_flags.origo = true;
            type->m_flags.invisible = true;
            return typeHandle;
        }

        if (typeData.type == NodeKind::origo) {
            type->m_flags.origo = true;
            type->m_flags.invisible = true;
        }
        else if (typeData.type == NodeKind::composite) {
            type->m_flags.composite = true;
            type->m_flags.invisible = true;
        }
        else
        {
            resolveMeshes(type, typeData);

            // NOTE KI container does not have mesh itself, but it can setup
            // material & program for contained nodes
            if (typeData.type != NodeKind::container) {
                if (!type->hasMesh()) {
                    KI_WARN(fmt::format(
                        "TYPE::SCENE_FILE_IGNORE: NO_MESH id={} ({})",
                        typeData.baseId, typeData.desc));
                    return pool::TypeHandle::NULL_HANDLE;
                }
            }

            resolveAddonMeshes(type, typeData);

            {
                // TODO KI this is WRONG, this should be per drawable mesh, not per type
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
        }

        {
            auto& flags = type->m_flags;
            for (auto& lodMesh : type->getLodMeshes()) {
                const auto& opt = lodMesh.m_drawOptions;
                flags.anySolid |= opt.isSolid();
                flags.anyAlpha |= opt.isAlpha();
                flags.anyBlend |= opt.isBlend();
                flags.anyAnimation |= lodMesh.m_flags.useAnimation;
            }
        }

        {
            type->setCustomMaterial(
                m_loaders->m_customMaterialLoader.createCustomMaterial(
                    typeData.customMaterial,
                    *m_loaders));
        }

        {
            const auto& scriptIds = m_loaders->m_scriptLoader.createScripts(
                typeData.scripts);

            for (auto& scriptId : scriptIds) {
                type->addScript(scriptId);
            }
        }

        type->m_cameraComponentDefinition = l.m_cameraLoader.createDefinition(typeData.camera);
        type->m_lightDefinition = l.m_lightLoader.createDefinition(typeData.light);

        if (!typeData.particleId.empty()) {
            auto* particleData = findParticle(typeData.particleId, particles);

            if (!particleData) {
                throw fmt::format("particle missing: type={}, type={}",
                    typeData.str(), typeData.particleId.str());
            }

            type->m_particleGeneratorDefinition = l.m_particleLoader.createDefinition(*particleData);
        }

        type->m_physicsDefinition = l.m_physicsLoader.createPhysicsDefinition(typeData.physics);

        type->m_generatorDefinition = l.m_generatorLoader.createGeneratorDefinition(
            typeData.generator,
            *m_loaders);

        if (!typeData.controllers.empty()) {
            type->m_controllerDefinitions = std::make_unique<std::vector<ControllerDefinition>>();
            for (auto& controllerData : typeData.controllers) {
                auto df = l.m_controllerLoader.createControllerDefinition(controllerData);
                if (!df) continue;

                type->m_controllerDefinitions->push_back(*df);
            }

            if (type->m_controllerDefinitions->empty()) {
                type->m_controllerDefinitions.reset();
            }
        }

        type->m_audioListenerDefinition = l.m_audioLoader.createListenerDefinition(typeData.audio.listener);
        type->m_audioSourceDefinitions = l.m_audioLoader.createSourceDefinitions(typeData.audio.sources);

        if (type->m_flags.text) {
            type->m_textGeneratorDefinition = m_loaders->m_textLoader.createDefinition(
                type,
                typeData.text,
                *m_loaders);
        }

        if (!typeData.compositeId.empty()) {
            const auto* composite = findComposite(typeData.compositeId, composites);

            if (!composite) {
                throw fmt::format("composite missing: type={}, type={}",
                    typeData.str(), typeData.compositeId.str());
            }

            type->m_compositeDefinition = l.m_compositeLoader.createCompositeDefinition(
                *composite,
                *m_loaders);
        }

        return typeHandle;
    }

    void NodeTypeBuilder::resolveMaterials(
        model::NodeType* type,
        mesh::LodMesh& lodMesh,
        const NodeTypeData& typeData,
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
                if (materialData.isAny()) {
                    KI_INFO_OUT(fmt::format(
                        "TYPE::MAT_ASSIGN: model={}, mesh={}, material={}, src={}, alias={}",
                        type->getName(),
                        lodMesh.getMeshName(),
                        material.m_name,
                        materialData.materialName,
                        materialData.aliasName));

                    material.assign(materialData.material);
                }
            }

            // ASSIGN - specific
            for (auto& materialData : meshData.materials) {
                if (materialData.match(material.m_name)) {
                    KI_INFO_OUT(fmt::format(
                        "TYPE::MAT_ASSIGN: model={}, mesh={}, material={}, src={}, alias={}",
                        type->getName(),
                        lodMesh.getMeshName(),
                        material.m_name,
                        materialData.materialName,
                        materialData.aliasName));

                    material.assign(materialData.material);
                }
            }

            // NOTE KI settings from mesh to material *AFTER* assign, *BEFORE* modify
            if (meshData.defaultPrograms) {
                material.m_defaultPrograms = true;
            }

            {
                for (const auto& srcIt : typeData.programs) {
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
                if (materialData.isAny()) {
                    KI_INFO_OUT(fmt::format(
                        "TYPE::MAT_MODIFY: model={}, mesh={}, material={}, src={}, alias={}",
                        type->getName(),
                        lodMesh.getMeshName(),
                        material.m_name,
                        materialData.materialName,
                        materialData.aliasName));

                    l.m_materialLoader.modifyMaterial(material, materialData);
                }
            }

            // MODIFY - specific material
            for (auto& materialData : meshData.materialModifiers) {
                if (materialData.match(material.m_name)) {
                    KI_INFO_OUT(fmt::format(
                        "TYPE::MAT_MODIFY: model={}, mesh={}, material={}, src={}, alias={}",
                        type->getName(),
                        lodMesh.getMeshName(),
                        material.m_name,
                        materialData.materialName,
                        materialData.aliasName));

                    l.m_materialLoader.modifyMaterial(material, materialData);
                }
            }

            // MODIFY - LOD
            if (lodData) {
                // MODIFY LOD - ANY
                for (auto& materialData : lodData->materialModifiers) {
                    const auto& alias = materialData.aliasName;
                    const auto& name = materialData.materialName;

                    if (alias == LOD_ALIAS_ANY)
                    {
                        KI_INFO_OUT(fmt::format(
                            "TYPE::MAT_MODIFY: model={}, mesh={}, material={}, name={}, alias={}",
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
                            "TYPE::MAT_MODIFY: model={}, mesh={}, material={}, name={}, alias={}",
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

    void NodeTypeBuilder::resolveMeshes(
        model::NodeType* type,
        const NodeTypeData& typeData)
    {
        for (const auto& meshData : typeData.meshes) {
            if (!meshData.enabled) continue;

            const auto startIndex = type->getLodMeshes().size();
            resolveMesh(type, typeData, meshData);
            const auto meshCount = type->getLodMeshes().size() - startIndex;

            if (meshCount > 0) {
                const auto& span = std::span{ type->modifyLodMeshes() }.subspan(startIndex, meshCount);
                std::set<const animation::Rig*> processedRigs;

                // process rigs
                for (auto& lodMesh : span) {
                    auto* mesh = lodMesh.getMesh<mesh::ModelMesh>();
                    if (!mesh) continue;

                    auto* rig = mesh->m_rig.get();
                    if (!rig) continue;

                    if (processedRigs.contains(rig)) continue;
                    processedRigs.insert(rig);

                    resolveRig(meshData, rig);
                }
            }
        }

        // NOTE KI ensure volume is containing all meshes
        type->prepareVolume();
    }

    void NodeTypeBuilder::resolveAddonMeshes(
        model::NodeType* type,
        const NodeTypeData& typeData)
    {
        for (const auto& meshData : typeData.availableAddons) {
            if (!meshData.enabled) continue;

            const auto startIndex = type->getLodMeshes().size();
            resolveMesh(type, typeData, meshData);
            const auto meshCount = type->getLodMeshes().size() - startIndex;

            if (meshCount > 0) {
                const auto& span = std::span{ type->modifyLodMeshes() }.subspan(startIndex, meshCount);
                // bind rigs
                for (auto& lodMesh : span) {
                    auto* mesh = lodMesh.getMesh<mesh::ModelMesh>();
                    if (!mesh) continue;

                    if (mesh->m_jointContainer && !meshData.useRig.empty()) {
                        std::shared_ptr<animation::Rig> rig;
                        for (const auto& lodMesh : type->getLodMeshes()) {
                            auto* rigMesh = lodMesh.getMesh<mesh::ModelMesh>();
                            if (!rigMesh) continue;

                            if (!rigMesh->m_rig) continue;

                            if (rigMesh->m_rig->matchAlias(meshData.useRig)) {
                                rig = rigMesh->m_rig;
                                break;
                            }
                        }

                        if (rig) {
                            mesh->m_rig = rig;
                            mesh->m_jointContainer->bindRig(*rig);
                        }
                    }
                }
            }
        }

        // NOTE KI volume does not include optional meshes
    }

    void NodeTypeBuilder::resolveMesh(
        model::NodeType* type,
        const NodeTypeData& typeData,
        const MeshData& meshData)
    {
        const auto& assets = Assets::get();

        size_t startIndex = type->getLodMeshes().size();
        size_t meshCount = 0;

        // NOTE KI materials MUST be resolved before loading mesh
        if (typeData.type == NodeKind::model) {
            type->m_flags.model = true;
            meshCount += resolveModelMesh(type, typeData, meshData);

            KI_INFO(fmt::format(
                "TYPE::SCENE_FILE_MESH: id={}, desc={}, type={}",
                typeData.baseId, typeData.desc, type->str()));
        }
        else if (typeData.type == NodeKind::text) {
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
        else if (typeData.type == NodeKind::terrain) {
            // NOTE KI handled via container + generator
            type->m_flags.terrain = true;
            throw std::runtime_error("Terrain not supported currently");
        }
        else if (typeData.type == NodeKind::container) {
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
                resolveLodMesh(type, typeData, meshData, lodMesh);
            }
        }
    }

    int NodeTypeBuilder::resolveModelMesh(
        model::NodeType* type,
        const NodeTypeData& typeData,
        const MeshData& meshData)
    {
        const auto& assets = Assets::get();
        int meshCount = 0;

        switch (meshData.type) {
        case MeshDataType::mesh: {
            auto future = MeshSetRegistry::get().getMeshSet(
                meshData.id,
                assets.modelsDir,
                meshData.path,
                meshData.smoothNormals,
                meshData.forceNormals);

            const auto& meshSet = future.get();

            if (meshSet) {
                meshCount += type->addMeshSet(*meshSet);

                KI_INFO_OUT(fmt::format(
                    "\n=======================\n[MESH_SET SUMMARY: {}]\n{}\n=======================",
                    meshSet->m_name,
                    meshSet->getSummary()));
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

    void NodeTypeBuilder::resolveLodMesh(
        model::NodeType* type,
        const NodeTypeData& typeData,
        const MeshData& meshData,
        mesh::LodMesh& lodMesh)
    {
        lodMesh.m_scale = meshData.scale;
        lodMesh.m_baseScale = meshData.baseScale;
        lodMesh.m_baseAxis = meshData.baseAxis;
        lodMesh.m_baseFront = meshData.baseFront;
        lodMesh.m_baseAdjust = meshData.baseAdjust;

        auto* lodData = resolveLod(type, typeData, meshData, lodMesh);

        assignMeshFlags(meshData.meshFlags, lodMesh.m_flags);
        if (lodData) {
            assignMeshFlags(lodData->meshFlags, lodMesh.m_flags);
        }

        resolveMaterials(type, lodMesh, typeData, meshData, lodData);
        lodMesh.setupDrawOptions();
    }

    const LodData* NodeTypeBuilder::resolveLod(
        model::NodeType* type,
        const NodeTypeData& typeData,
        const MeshData& meshData,
        mesh::LodMesh& lodMesh)
    {
        auto* mesh = lodMesh.getMesh<mesh::Mesh>();
        if (!mesh) return nullptr;

        lodMesh.m_priority = typeData.priority;

        const auto* lod = meshData.findLod(mesh->m_name);
        if (!lod) {
            // NOTE KI if lods defined then default to 0 mask
            if (!meshData.lods.empty()) {
                KI_WARN_OUT(fmt::format("TYPE::SCENE: MISSING_LOD : mesh={}", mesh->m_name));
                //lodMesh.m_levelMask = 0;
            }
            return nullptr;
        }

        lodMesh.m_minDistance2 = lod->minDistance * lod->minDistance;
        lodMesh.m_maxDistance2 = lod->maxDistance * lod->maxDistance;
        lodMesh.m_priority = lod->priority != 0 ? lod->priority : typeData.priority;

        return lod;
    }

    void NodeTypeBuilder::resolveRig(
        const MeshData& meshData,
        animation::Rig* rig)
    {
        // Setup alias
        for (const auto& rigData : meshData.rigs) {
            if (rigData.isAny()) continue;
            if (!rigData.match(rig->getName())) continue;
            rig->setAlias(rigData.alias);
        }

        resolveSockets(
            meshData,
            rig);

        resolveAnimations(
            meshData,
            rig);

        rig->dump();
    }

    void NodeTypeBuilder::resolveSockets(
        const MeshData& meshData,
        animation::Rig* rig)
    {
        if (!rig) return;

        for (const auto& rigData : meshData.rigs) {
            if (!rigData.match(rig->getName())) continue;

            for (const auto& socketData : rigData.sockets) {
                if (!socketData.enabled) continue;

                // TODO KI scale is in LodMesh level, but sockets in Mesh level
                // => PROBLEM if same mesh is used for differently scaled LodMeshes
                //glm::vec3 meshScale{ 0.01375f * 2.f };
                auto meshScale = meshData.scale * meshData.baseScale;

                animation::RigSocket socket{
                    socketData.name,
                    socketData.joint,
                    socketData.offset.toTransform(),
                    meshScale
                };
                socket.m_role = socketData.role;

                auto socketIndex = rig->registerSocket(socket);
                if (socketIndex < 0) {
                    KI_WARN_OUT(fmt::format(
                        "TYPE::SCENE_ERROR: SOCKET_NOT_FOUND - rig={}, node={}, socket={}",
                        rig->getName(), socket.m_jointName, socket.m_name));
                }
            }
        }
        rig->prepareSockets();
    }

    void NodeTypeBuilder::resolveAnimations(
        const MeshData& meshData,
        animation::Rig* rig)
    {
        const auto& assets = Assets::get();

        if (!rig) return;

        for (const auto& rigData : meshData.rigs) {
            if (!rigData.match(rig->getName())) continue;

            for (auto& animationData : rigData.animations) {
                loadAnimation(
                    assets.modelsDir,
                    meshData.baseDir,
                    *rig,
                    animationData);
            }
        }
    }

    void NodeTypeBuilder::loadAnimation(
        const std::string rootDir,
        const std::string& baseDir,
        animation::Rig & rig,
        const AnimationData & data)
    {
        const auto& assets = Assets::get();

        mesh_set::AnimationImporter importer{};
        std::string filePath;

        if (!data.path.empty()) {
            {
                filePath = util::joinPathExt(
                    rootDir,
                    baseDir,
                    data.path, "");
            }

            if (!util::fileExists(filePath)) {
                filePath = util::joinPath(
                    rootDir,
                    data.path);
            }
        }

        if (!data.path.empty()) {
            importer.loadAnimations(
                rig,
                data.name,
                filePath);
        }

        try {
            // map clips
            for (const auto& clipData : data.clips) {
                const auto& uniqueName = clipData.getUniqueName(data.name);
                auto* clip = rig.modifyClipContainer().findClipByUniqueName(uniqueName);
                if (clip) {
                    clip->m_id = SID(clipData.name);
                }
                else {
                    KI_WARN_OUT(fmt::format("TYPE::SCENE_ERROR: CLIP_NOT_FOUND - clip={}, uniq={}",
                        clipData.name, uniqueName));
                }
            }
        }
        catch (mesh_set::AnimationNotFoundError ex) {
            KI_CRITICAL(fmt::format("TYPE::SCENE_ERROR: LOAD_ANIMATION - {}", ex.what()));
            //throw std::current_exception();
        }
    }

    void NodeTypeBuilder::assignTypeFlags(
        const NodeTypeData& typeData,
        TypeFlags& flags)
    {
        const auto& container = typeData.typeFlags;

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
            flags.physics = typeData.physics.enabled;
        }

        flags.navMesh = container.getFlag("nav_mesh", flags.navMesh);
        flags.navPhysics = container.getFlag("nav_physics", flags.navPhysics);
    }

    void NodeTypeBuilder::assignMeshFlags(
        const FlagContainer& container,
        mesh::MeshFlags& flags)
    {
        flags.billboard = container.getFlag("billboard", flags.billboard);
        flags.tessellation = container.getFlag("tessellation", flags.tessellation);

        flags.preDepth = container.getFlag("pre_depth", flags.preDepth);

        flags.noVolume = container.getFlag("no_volume", flags.noVolume);

        {
            flags.useJoints = container.getFlag("use_joints", flags.useJoints);
            flags.useJoints = container.getFlag("use_bones", flags.useJoints);

            // NOTE KI Joints are *required* if using animation
            flags.useAnimation = container.getFlag("use_animation", flags.useAnimation);
            if (flags.useAnimation) {
                flags.useJoints = true;
            }

            // NOTE KI no Joints debug if no Joints
            flags.useJointsDebug = container.getFlag("use_joints_debug", flags.useJointsDebug);
            flags.useJointsDebug = container.getFlag("use_bones_debug", flags.useJointsDebug);

            if (!flags.useJoints) {
                flags.useJointsDebug = false;
            }
        }

        flags.clip = container.getFlag("clip", flags.clip);
    }
}
