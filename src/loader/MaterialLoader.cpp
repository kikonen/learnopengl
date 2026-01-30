#include "MaterialLoader.h"

#include <filesystem>

#include "asset/Assets.h"

#include "util/Log.h"
#include "util/util.h"
#include "util/file.h"

#include "ki/sid.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshFlags.h"

#include "shader/ProgramRegistry.h"

#include "loader/converter/YamlConverter.h"

#include "loader/document.h"
#include "Loaders.h"
#include "loader_util.h"

#include "Context.h"


namespace {
    constexpr float DEF_ALPHA = 1.0f;

    const std::vector<std::regex> texturesMatchers{
        std::regex("textures"),
    };

    const std::vector<std::regex> hdriMatchers{
        std::regex(".*[\\.]hdr"),
    };

}

namespace loader {
    MaterialLoader::MaterialLoader(
        const std::shared_ptr<Context>& ctx)
        : BaseLoader(ctx)
    {
    }

    void MaterialLoader::loadMaterialModifiers(
        const loader::DocNode& node,
        std::vector<MaterialData>& materials,
        Loaders& loaders) const
    {
        for (const auto& entry : node.getNodes()) {
            MaterialData& data = materials.emplace_back();
            loadMaterialModifier(entry, data, loaders);
        }
    }

    void MaterialLoader::loadMaterialModifier(
        const loader::DocNode& node,
        MaterialData& data,
        Loaders& loaders) const
    {
        data.enabled = true;
        data.aliasName = MATERIAL_ALIAS_ANY;

        loadMaterial(node, data, loaders);

        data.material.m_name = "<modifier>";
        data.materialName = data.material.m_name;
    }

    void MaterialLoader::loadMaterials(
        const loader::DocNode& node,
        std::vector<MaterialData>& materials,
        Loaders& loaders) const
    {
        for (const auto& entry : node.getNodes()) {
            MaterialData& data = materials.emplace_back();
            loadMaterial(entry, data, loaders);
            data.materialName = data.material.m_name;

            if (data.materialName.empty() && data.aliasName != MATERIAL_ALIAS_ANY)
            {
                data.materialName = data.aliasName;
            }
        }
    }

    void MaterialLoader::loadMaterial(
        const loader::DocNode& node,
        MaterialData& data,
        Loaders& loaders) const
    {
        Material& material = data.material;
        auto& fields = data.fields;

        loadMaterialPrefab(node.findNode("prefab"), data, loaders);

        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::DocNode& v = pair.getNode();

            const auto k = util::toLower(key);

            if (k == "prefab") {
                continue;
            }

            if (k == "name") {
                material.m_name = readString(v);
            }
            else if (k == "alias") {
                data.aliasName = readString(v);
            }
            else if (k == "base" || k == "base_dir") {
                fields.baseDir = true;
                material.m_baseDir = readString(v);
            }
            //else if (k == "ns") {
            //    material.ns = readFloat(v);
            //    fields.ns = true;
            //}
            //else if (k == "ka") {
            //    material.ka = readRGB(v);
            //    fields.ka = true;
            //}
            else if (k == "kd") {
                material.kd = readRGBA(v);
                fields.kd = true;
            }
            //else if (k == "ks") {
            //    material.ks = readRGB(v);
            //    fields.ks = true;
            //}
            else if (k == "ke") {
                material.ke = glm::vec4(readRGB(v), 1.f);
                fields.ke = true;
            }
            //else if (k == "ni") {
            //    material.ni = readFloat(v);
            //    fields.ni = true;
            //}
            //else if (k == "d") {
            //    material.d = readFloat(v);
            //    fields.d = true;
            //}
            //else if (k == "illum") {
            //    material.d = readFloat(v);
            //    fields.illum = true;
            //}
            else if (k == "map_kd") {
                std::string line = readString(v);
                material.addTexture(
                    TextureType::diffuse,
                    line,
                    true);
            }
            else if (k == "map_ke") {
                std::string line = readString(v);
                material.addTexture(
                    TextureType::emission,
                    line,
                    true);
            }
            else if (k == "map_ks") {
                std::string line = readString(v);
                material.addTexture(
                    TextureType::specular,
                    line,
                    true);
            }
            else if (k == "map_normal" || k == "map_no") {
                // NOTE KI bump is NOT saame thing
                // k == "map_bump" ||
                std::string line = readString(v);
                material.addTexture(
                    TextureType::map_normal,
                    line,
                    true);
            }
            else if (k == "map_bump_strength") {
                material.map_bump_strength = readFloat(v);
                fields.map_bump_strength = true;
            }
            else if (k == "map_dudv") {
                std::string line = readString(v);
                material.addTexture(
                    TextureType::map_dudv,
                    line,
                    true);
            }
            else if (k == "map_noise") {
                std::string line = readString(v);
                material.addTexture(
                    TextureType::map_noise,
                    line,
                    true);
            }
            else if (k == "map_noise_2") {
                std::string line = readString(v);
                material.addTexture(
                    TextureType::map_noise_2,
                    line,
                    true);
            }
            else if (k == "map_displacement" || k == "map_di") {
                std::string line = readString(v);
                material.addTexture(
                    TextureType::map_displacement,
                    line,
                    true);
            }
            else if (k == "map_opacity") {
                std::string line = readString(v);
                material.addTexture(
                    TextureType::map_opacity,
                    line,
                    true);
            }
            else if (k == "map_custom_1") {
                std::string line = readString(v);
                material.addTexture(
                    TextureType::map_custom_1,
                    line,
                    true);
            }
            else if (k == "map_mras") {
                std::string line = readString(v);
                material.addTexture(
                    TextureType::map_mras,
                    line,
                    true);
            }
            else if (k == "mras") {
                // NOTE KI last one is *specular* (not opacity)
                auto vec = readFloatVector(v, 4);
                while (vec.size() < 4) {
                    vec.push_back(0.f);
                }
                material.mras = glm::vec4(vec[0], vec[1], vec[2], vec[3]);
                fields.mras = true;
            }
            else if (k == "invert_occlusion") {
                material.m_invertOcclusion = readBool(v);
                fields.invertOcclusion = true;
            }
            else if (k == "invert_metalness") {
                material.m_invertMetalness = readBool(v);
                fields.invertMetalness = true;
            }
            else if (k == "invert_roughness") {
                material.m_invertRoughness = readBool(v);
                fields.invertRoughness = true;
            }
            else if (k == "pattern") {
                material.pattern = readInt(v);
                fields.pattern = true;
            }
            else if (k == "reflection") {
                material.reflection = readFloat(v);
                fields.reflection = true;
            }
            else if (k == "refraction") {
                material.refraction = readFloat(v);
                fields.refraction = true;
            }
            else if (k == "refraction_ratio") {
                auto ratio = readRefractionRatio(v);
                material.refractionRatio = convertRefractionRatio(ratio);
                fields.refractionRatio = true;
            }
            else if (k == "tiling") {
                float tiling = readFractional(v);
                material.tilingX = tiling;
                material.tilingY = tiling;
                fields.tilingX = true;
                fields.tilingY = true;
            }
            else if (k == "tiling_x") {
                material.tilingX = readFractional(v);
                fields.tilingX = true;
            }
            else if (k == "tiling_y") {
                material.tilingY = readFractional(v);
                fields.tilingY = true;
            }
            else if (k == "sprites") {
                material.spriteCount = readInt(v);
                fields.spriteCount = true;
            }
            else if (k == "sprites_x") {
                material.spritesX = readInt(v);
                fields.spritesX = true;
            }
            else if (k == "layers") {
                material.layers = readInt(v);
                fields.layers = true;
            }
            else if (k == "layers_depth") {
                material.layersDepth = readFloat(v);
                fields.layersDepth = true;
            }
            else if (k == "parallax_depth" || k == "parallax") {
                material.parallaxDepth = readFloat(v);
                fields.parallaxDepth = true;
            }
            else if (k == "texture_spec") {
                loadTextureSpec(v, material.textureSpec);
                fields.textureSpec = true;
            }
            else if (k == "alpha") {
                material.alpha = readBool(v);
                fields.alpha = true;
            }
            else if (k == "blend") {
                material.blend = readBool(v);
                fields.blend = true;
            }
            else if (k == "render_back") {
                material.renderBack = readBool(v);
                fields.renderBack = true;
            }
            else if (k == "line_mode") {
                material.lineMode = readBool(v);
                fields.lineMode = true;
            }
            else if (k == "reverse_front_face") {
                material.reverseFrontFace = readBool(v);
                fields.reverseFrontFace = true;
            }
            else if (k == "no_depth") {
                material.noDepth = readBool(v);
                fields.noDepth = true;
            }
            else if (k == "default_programs" || k == "default_program") {
                material.m_defaultPrograms = readBool(v);
                fields.defaultPrograms = true;
            }
            else if (k == "program" || k == "shader") {
                material.m_programNames[MaterialProgramType::shader] = readString(v);
            }
            else if (k == "oit_program") {
                material.m_programNames[MaterialProgramType::oit] = readString(v);
            }
            else if (k == "shadow_program") {
                material.m_programNames[MaterialProgramType::shadow] = readString(v);
            }
            else if (k == "pre_depth_program") {
                material.m_programNames[MaterialProgramType::pre_depth] = readString(v);
            }
            else if (k == "selection_program") {
                material.m_programNames[MaterialProgramType::selection] = readString(v);
            }
            else if (k == "id_program") {
                material.m_programNames[MaterialProgramType::object_id] = readString(v);
            }
            else if (k == "normal_program") {
                material.m_programNames[MaterialProgramType::normal] = readString(v);
            }
            else if (k == "geometry_type") {
                material.m_geometryType = readString(v);
            }
            else if (k == "shared_definitions") {
                for (const auto& defNode : v.getNodes()) {
                    const auto& defName = defNode.getName();
                    const auto& defValue = readString(defNode.getNode());
                    material.m_sharedDefinitions[util::toUpper(defName)] = defValue;
                }
            }
            else if (k == "program_definitions") {
                for (const auto& defNode : v.getNodes()) {
                    const auto& defName = defNode.getName();
                    const auto& defValue = readString(defNode.getNode());
                    material.m_programDefinitions[util::toUpper(defName)] = defValue;
                }
            }
            else if (k == "oit_definitions") {
                for (const auto& defNode : v.getNodes()) {
                    const auto& defName = defNode.getName();
                    const auto& defValue = readString(defNode.getNode());
                    material.m_oitDefinitions[util::toUpper(defName)] = defValue;
                }
            }
            else if (k == "shadow_definitions") {
                for (const auto& defNode : v.getNodes()) {
                    const auto& defName = defNode.getName();
                    const auto& defValue = readString(defNode.getNode());
                    material.m_shadowDefinitions[util::toUpper(defName)] = defValue;
                }
            }
            else if (k == "selection_definitions") {
                for (const auto& defNode : v.getNodes()) {
                    const auto& defName = defNode.getName();
                    const auto& defValue = readString(defNode.getNode());
                    material.m_selectionDefinitions[util::toUpper(defName)] = defValue;
                }
            }
            else if (k == "id_definitions") {
                for (const auto& defNode : v.getNodes()) {
                    const auto& defName = defNode.getName();
                    const auto& defValue = readString(defNode.getNode());
                    material.m_idDefinitions[util::toUpper(defName)] = defValue;
                }
            }
            else if (k == "normal_definitions") {
                for (const auto& defNode : v.getNodes()) {
                    const auto& defName = defNode.getName();
                    const auto& defValue = readString(defNode.getNode());
                    material.m_normalDefinitions[util::toUpper(defName)] = defValue;
                }
            }
            else if (k == "updater_id") {
                material.m_updaterId = SID(readString(v));
            }
            else {
                reportUnknown("material_entry", k, v);
            }
        }

        if (material.m_name == "female_elf_hooded_pants_set")
            int x = 0;

        if (material.hasRegisteredTex(TextureType::map_displacement)) {
            if (!fields.parallaxDepth) {
                const auto& assets = Assets::get();
                KI_INFO_OUT(fmt::format("LOADER_MATERIAL: apply_default_parallax={}", assets.parallaxDepth));
                material.parallaxDepth = assets.parallaxDepth;
                fields.parallaxDepth = true;
            }
        }
    }

    void MaterialLoader::loadMaterialPrefab(
        const loader::DocNode& node,
        MaterialData& data,
        Loaders& loaders) const
    {
        if (node.isNull()) return;

        std::string path = readString(node);
        if (path.empty()) return;

        {
            std::filesystem::path filePath{ path };
            if (filePath.extension().empty()) {
                path += ".yml";
            }
        }

        std::string fullPath = path;

        if (!util::fileExists(fullPath)) {
            fullPath = util::joinPath(m_ctx->m_assetsDir, path);
        }

        if (!util::fileExists(fullPath)) {
            fullPath = util::joinPath(m_ctx->m_dirName, path);
        }

        KI_INFO_OUT(fmt::format("material_prefab={}", fullPath));

        if (!util::fileExists(fullPath))
        {
            throw fmt::format("INVALID: material_prefab missing - path={}", fullPath);
        }

        loader::YamlConverter converter;
        auto doc = converter.load(fullPath);

        for (const auto& pair : doc.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "material") {
                std::vector<NodeData> clones;
                loadMaterial(
                    v,
                    data,
                    loaders);
            }
        }
    }

    void MaterialLoader::loadTextureSpec(
        const loader::DocNode& node,
        TextureSpec& textureSpec) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "wrap") {
                loadTextureWrap(k, v, textureSpec.wrapS);
                loadTextureWrap(k, v, textureSpec.wrapT);
            }
            else if (k == "wrap_s") {
                loadTextureWrap(k, v, textureSpec.wrapS);
            }
            else if (k == "wrap_t") {
                loadTextureWrap(k, v, textureSpec.wrapT);
            }
            else {
                reportUnknown("tex_spec", k, v);
            }
        }
    }

    void MaterialLoader::loadTextureWrap(
        const std::string& k,
        const loader::DocNode& v,
        uint16_t& wrapMode) const
    {
        const std::string& wrap = readString(v);
        if (wrap == "GL_REPEAT") {
            wrapMode = GL_REPEAT;
        }
        else if (wrap == "GL_CLAMP_TO_EDGE") {
            wrapMode = GL_CLAMP_TO_EDGE;
        }
        else {
            // NOTE KI GL_REPEAT is GL default
            wrapMode = GL_REPEAT;
            reportUnknown("wrap_mode", k, v);
        }
    }

    void MaterialLoader::modifyMaterial(
        Material& m,
        const MaterialData& data)
    {
        if (!data.enabled) {
            return;
        }

        const MaterialField& f = data.fields;
        const Material & mod = data.material;

        if (f.textureSpec) m.textureSpec = mod.textureSpec;

        if (f.baseDir) m.m_baseDir = mod.m_baseDir;
        if (f.geometryType) m.m_geometryType = mod.m_geometryType;

        if (f.pattern) m.pattern = mod.pattern;
        if (f.reflection) m.reflection = mod.reflection;
        if (f.refraction) m.refraction = mod.refraction;
        if (f.refractionRatio) m.refractionRatio = mod.refractionRatio;

        if (f.tilingX) m.tilingX = mod.tilingX;
        if (f.tilingY) m.tilingY = mod.tilingY;

        if (f.spriteCount) m.spriteCount = mod.spriteCount;
        if (f.spritesX) m.spritesX = mod.spritesX;

        //if (f.ns) m.ns = mod.ns;

        //if (f.ka) m.ka = mod.ka;

        if (f.kd) m.kd = mod.kd;
        //if (f.ks) m.ks = mod.ks;
        if (f.ke) m.ke = mod.ke;

        if (f.map_bump_strength) m.map_bump_strength = mod.map_bump_strength;

        //if (f.ni) m.ni = mod.ni;
        //if (f.d) m.d = mod.d;
        //if (f.illum) m.illum = mod.illum;

        if (f.layers) m.layers = mod.layers;
        if (f.layersDepth) m.layersDepth = mod.layersDepth;
        if (f.parallaxDepth) m.parallaxDepth = mod.parallaxDepth;

        if (f.mras) m.mras = mod.mras;

        if (f.invertOcclusion) m.m_invertOcclusion = mod.m_invertOcclusion;
        if (f.invertMetalness) m.m_invertMetalness = mod.m_invertMetalness;
        if (f.invertRoughness) m.m_invertRoughness = mod.m_invertRoughness;

        if (f.alpha) m.alpha = mod.alpha;
        if (f.blend) m.blend = mod.blend;

        if (f.renderBack) m.renderBack = mod.renderBack;
        if (f.lineMode) m.lineMode = mod.lineMode;
        if (f.reverseFrontFace) m.reverseFrontFace = mod.reverseFrontFace;
        if (f.noDepth) m.noDepth = mod.noDepth;

        if (f.gbuffer) m.gbuffer = mod.gbuffer;

        if (f.defaultPrograms) m.m_defaultPrograms = mod.m_defaultPrograms;

        for (const auto& [type, info] : mod.getTextures()) {
            m.addTexture(type, info.path, info.compressed);
        }

        for (const auto& progIt : mod.m_programNames) {
            m.m_programNames[progIt.first] = progIt.second;
        }

        for (const auto& progIt : mod.m_sharedDefinitions) {
            m.m_sharedDefinitions[progIt.first] = progIt.second;
        }

        for (const auto& progIt : mod.m_programDefinitions) {
            m.m_programDefinitions[progIt.first] = progIt.second;
        }

        for (const auto& progIt : mod.m_oitDefinitions) {
            m.m_oitDefinitions[progIt.first] = progIt.second;
        }

        for (const auto& progIt : mod.m_shadowDefinitions) {
            m.m_shadowDefinitions[progIt.first] = progIt.second;
        }

        for (const auto& progIt : mod.m_selectionDefinitions) {
            m.m_selectionDefinitions[progIt.first] = progIt.second;
        }

        for (const auto& progIt : mod.m_idDefinitions) {
            m.m_idDefinitions[progIt.first] = progIt.second;
        }

        for (const auto& progIt : mod.m_normalDefinitions) {
            m.m_normalDefinitions[progIt.first] = progIt.second;
        }
    }

    void MaterialLoader::resolveMaterial(
        const mesh::MeshFlags& meshFlags,
        Material& material)
    {
        {
            const auto& shaderName = selectProgram(
                MaterialProgramType::shader,
                material.m_programNames,
                material.m_defaultPrograms ? SHADER_G_TEX : "");

            if (shaderName.starts_with("g_")) {
                material.gbuffer = true;
            }

            if (material.blend) {
                // NOTE KI alpha MUST BE true if blend
                material.alpha = true;
            }
        }

        material.loadTextures();
        resolveProgram(meshFlags, material);

        {
            bool useParallax = material.hasBoundTex(TextureType::map_displacement) && material.parallaxDepth > 0;
            if (!useParallax) {
                material.parallaxDepth = 0.f;
            }
        }
    }

    void MaterialLoader::resolveProgram(
        const mesh::MeshFlags& meshFlags,
        Material& material)
    {
        const auto& assets = Assets::get();

        const bool useDudvTex = material.hasBoundTex(TextureType::map_dudv);
        const bool useDisplacementTex = material.hasBoundTex(TextureType::map_displacement);
        const bool useNormalTex = material.hasBoundTex(TextureType::map_normal);
        const bool useCubeMap = 1.0 - material.reflection - material.refraction < 1.0;
        const bool useNormalPattern = material.pattern > 0;
        const bool useParallax = material.hasBoundTex(TextureType::map_displacement) && material.parallaxDepth > 0;

        const bool useTBN = useNormalTex || useDudvTex || useDisplacementTex;

        const auto& shaderName = selectProgram(
            MaterialProgramType::shader,
            material.m_programNames,
            material.m_defaultPrograms ? SHADER_G_TEX : "");

        auto preDepthName = selectProgram(
            MaterialProgramType::pre_depth,
            material.m_programNames,
            SHADER_PRE_DEPTH_PASS);

        const auto& oitName = selectProgram(
            MaterialProgramType::oit,
            material.m_programNames,
            material.m_defaultPrograms ? SHADER_OIT_PASS : "");

        const auto& shadowName = selectProgram(
            MaterialProgramType::shadow,
            material.m_programNames,
            material.m_defaultPrograms ? SHADER_SHADOW : "");

        const auto& selectionName = selectProgram(
            MaterialProgramType::selection,
            material.m_programNames,
            SHADER_SELECTION);

        const auto& objectIdName = selectProgram(
            MaterialProgramType::object_id,
            material.m_programNames,
            SHADER_OBJECT_ID);

        const auto& normalName = selectProgram(
            MaterialProgramType::normal,
            material.m_programNames,
            SHADER_NORMAL);

        if (!shaderName.empty()) {
            std::map<std::string, std::string, std::less<>> definitions;
            std::map<std::string, std::string, std::less<>> oitDefinitions;
            std::map<std::string, std::string, std::less<>> shadowDefinitions;
            std::map<std::string, std::string, std::less<>> selectionDefinitions;
            std::map<std::string, std::string, std::less<>> idDefinitions;
            std::map<std::string, std::string, std::less<>> normalDefinitions;

            for (const auto& [k, v] : material.m_sharedDefinitions) {
                definitions[k] = v;
                oitDefinitions[k] = v;
                shadowDefinitions[k] = v;
                selectionDefinitions[k] = v;
                idDefinitions[k] = v;
                normalDefinitions[k] = v;
            }

            for (const auto& [k, v] : material.m_programDefinitions) {
                definitions[k] = v;
            }

            for (const auto& [k, v] : material.m_oitDefinitions) {
                oitDefinitions[k] = v;
            }

            // NOTE KI *NOT* same as program, to allow maximal reuse of shadow program
            // i.e. most defs don't affect shadow
            for (const auto& [k, v] : material.m_shadowDefinitions) {
                shadowDefinitions[k] = v;
            }

            for (const auto& [k, v] : material.m_selectionDefinitions) {
                selectionDefinitions[k] = v;
            }

            for (const auto& [k, v] : material.m_idDefinitions) {
                idDefinitions[k] = v;
            }

            for (const auto& [k, v] : material.m_normalDefinitions) {
                normalDefinitions[k] = v;
            }

            std::map<std::string, std::string, std::less<>> preDepthDefinitions;

            bool usePreDepth = meshFlags.preDepth;
            bool useJoints = meshFlags.useJoints;
            //bool useSockets = true; // meshFlags.useSockets;
            bool useDebug = assets.glslUseDebug;

            if (material.alpha) {
                definitions[DEF_USE_ALPHA] = "1";
                //oitDefinitions[DEF_USE_ALPHA] = "1";
                shadowDefinitions[DEF_USE_ALPHA] = "1";
                selectionDefinitions[DEF_USE_ALPHA] = "1";
                idDefinitions[DEF_USE_ALPHA] = "1";
                usePreDepth = false;
            }
            if (material.blend) {
                definitions[DEF_USE_BLEND] = "1";
                usePreDepth = false;
            }

            if (useTBN) {
                definitions[DEF_USE_TBN] = "1";
            }
            //if (useDudvTex) {
            //    definitions[DEF_USE_DUDV_TEX] = "1";
            //}
            //if (useDisplacementTex) {
            //    definitions[DEF_USE_DISPLACEMENT_TEX] = "1";
            //}
            if (useNormalTex) {
                definitions[DEF_USE_NORMAL_TEX] = "1";
            }
            if (useParallax) {
                definitions[DEF_USE_PARALLAX] = "1";
            }
            if (useCubeMap) {
                definitions[DEF_USE_CUBE_MAP] = "1";
            }
            if (useNormalPattern) {
                definitions[DEF_USE_NORMAL_PATTERN] = "1";
            }
            if (useJoints) {
                definitions[DEF_USE_JOINTS] = "1";
                oitDefinitions[DEF_USE_JOINTS] = "1";
                shadowDefinitions[DEF_USE_JOINTS] = "1";
                selectionDefinitions[DEF_USE_JOINTS] = "1";
                idDefinitions[DEF_USE_JOINTS] = "1";
                normalDefinitions[DEF_USE_JOINTS] = "1";
            }
            //if (useSockets) {
            //    definitions[DEF_USE_SOCKETS] = "1";
            //    oitDefinitions[DEF_USE_SOCKETS] = "1";
            //    shadowDefinitions[DEF_USE_SOCKETS] = "1";
            //    selectionDefinitions[DEF_USE_SOCKETS] = "1";
            //    idDefinitions[DEF_USE_SOCKETS] = "1";
            //    normalDefinitions[DEF_USE_SOCKETS] = "1";
            //}
            if (useDebug) {
                definitions[DEF_USE_DEBUG] = "1";
            }

            material.m_programs[MaterialProgramType::shader] = ProgramRegistry::get().getProgramId(
                shaderName,
                false,
                material.m_geometryType,
                definitions);

            if (!oitName.empty()) {
                material.m_programs[MaterialProgramType::oit] = ProgramRegistry::get().getProgramId(
                    oitName,
                    false,
                    "",
                    oitDefinitions);
            }

            if (!shadowName.empty()) {
                //{
                //    size_t shadowCount = std::min(
                //        std::max(Assets::get().shadowPlanes.size() - 1, static_cast<size_t>(1)),
                //        static_cast<size_t>(MAX_SHADOW_MAP_COUNT_ABS));

                //    shadowDefinitions[DEF_MAX_SHADOW_MAP_COUNT] = std::to_string(shadowCount);
                //}

                material.m_programs[MaterialProgramType::shadow] = ProgramRegistry::get().getProgramId(
                    shadowName,
                    false,
                    "",
                    shadowDefinitions);
            }

            if (usePreDepth) {
                material.m_programs[MaterialProgramType::pre_depth] = ProgramRegistry::get().getProgramId(
                    preDepthName,
                    false,
                    "",
                    preDepthDefinitions);
            }

            if (!selectionName.empty()) {
                material.m_programs[MaterialProgramType::selection] = ProgramRegistry::get().getProgramId(
                    selectionName,
                    false,
                    "",
                    selectionDefinitions);
            }

            if (!objectIdName.empty()) {
                material.m_programs[MaterialProgramType::object_id] = ProgramRegistry::get().getProgramId(
                    objectIdName,
                    false,
                    "",
                    idDefinitions);
            }

            if (!normalName.empty()) {
                material.m_programs[MaterialProgramType::normal] = ProgramRegistry::get().getProgramId(
                    normalName,
                    false,
                    "",
                    normalDefinitions);
            }
        }
    }

    std::string MaterialLoader::selectProgram(
        MaterialProgramType type,
        const std::map<MaterialProgramType, std::string> programs,
        const std::string& defaultValue)
    {
        std::string program;
        bool found = false;
        const auto& it = programs.find(type);
        if (it != programs.end()) {
            program = it->second;
            found = true;
        }
        return found ? program : defaultValue;
    }
}
