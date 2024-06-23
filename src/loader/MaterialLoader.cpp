#include "MaterialLoader.h"

#include <filesystem>

#include "asset/Assets.h"

#include "util/Log.h"
#include "util/Util.h"

#include "mesh/MeshType.h"

#include "registry/ProgramRegistry.h"

#include "loader/document.h"


namespace {
    const float DEF_ALPHA = 1.0;

    const std::vector<std::regex> texturesMatchers{
        std::regex("textures"),
    };

    const std::vector<std::regex> hdriMatchers{
        std::regex(".*[\\.]hdr"),
    };

    const std::vector<std::regex> ignoreMatchers{
        std::regex(".*nope.*"),
        std::regex(".*[\\.]blend"),
        std::regex(".*[\\.]exr"),
        std::regex(".*[\\.]txt"),
        std::regex(".*[\\.]usda"),
        std::regex(".*preview.*"),
        std::regex(".*normaldx.*"),
        std::regex(".*bc_neutral.*"),

        std::regex(".*_micron[\\.].*"),
        std::regex(".*_micronmask[\\.].*"),
        std::regex(".*_resourcemap_position[\\.].*"),
        std::regex(".*_resourcemap_wsnormal[\\.].*"),
        std::regex(".*_sssmap[\\.].*"),
        std::regex(".*_transmap[\\.].*"),
    };

    const std::vector<std::regex> imageMatchers{
        std::regex(".*[\\.]hdr"),
        std::regex(".*[\\.]png"),
        std::regex(".*[\\.]jpg"),
        std::regex(".*[\\.]tga"),
    };

    const std::vector<std::regex> ktxMatchers{
        std::regex(".*[\\.]ktx"),
    };

    const std::vector<std::regex> colorMatchers{
        std::regex(".*[-_ ]color[-_ \\.].*"),
        std::regex(".*[-_ ]col[-_ \\.].*"),
        std::regex(".*[-_ ]basecolor[-_ \\.].*"),
        std::regex(".*[-_ ]diff[-_ \\.].*"),
        std::regex(".*[-_ ]alb[-_ \\.].*"),
        std::regex(".*[-_ ]albedo[-_ \\.].*"),
        std::regex(".*[-_ ]albedoopacity[-_ \\.].*"),
        std::regex(".*[-_ ]albedotransparency[-_ \\.].*"),
        std::regex(".*[-_ ]basecoloralpha[-_ \\.].*"),
        std::regex(".*[-_ ]a[\\.].*"),
        std::regex(".*[-_ ]c[\\.].*"),
        std::regex(".*[-_ ]bc[\\.].*"),
        std::regex(".*[-_ ]a_m[\\.].*"),
        std::regex(".*[-_ ]b[\\.].*"),
    };

    const std::vector<std::regex> emissionMatchers{
        std::regex(".*[-_ ]emission[-_ \\.].*"),
        std::regex(".*[-_ ]emi[-_ \\.].*"),
        std::regex(".*[-_ ]emissive[-_ \\.].*"),
    };

    const std::vector<std::regex> normalMatchers{
        std::regex(".*[-_ ]normal[-_ \\.].*"),
        std::regex(".*[-_ ]normals[-_ \\.].*"),
        std::regex(".*[-_ ]normalgl[-_ \\.].*"),
        std::regex(".*[-_ ]nrm[-_ \\.].*"),
        std::regex(".*[-_ ]nor[-_ \\.].*"),
        std::regex(".*[-_ ]nor[-_ \\.].*"),
        std::regex(".*[-_ ]nml[-_ \\.].*"),
        std::regex(".*[-_ ]n[\\.].*"),
    };

    const std::vector<std::regex> metalnessMatchers{
        std::regex(".*[-_ ]metalness[-_ \\.].*"),
        std::regex(".*[-_ ]met[-_ \\.].*"),
        std::regex(".*[-_ ]metallic[-_ \\.].*"),
        // TODO KI logic various random combined texture formats
        std::regex(".*[-_ ]metallicsmoothness[-_ \\.].*"),
        //std::regex(".*[-_ ]occlusionroughnessmetallic[-_ \\.].*"),
        //std::regex(".*[-_ ]aorm[\\.].*"),
        //std::regex(".*[-_ ]rom[\\.].*"),
    };

    const std::vector<std::regex> roughnessMatchers{
        std::regex(".*[-_ ]roughness[-_ \\.].*"),
        std::regex(".*[-_ ]rough[-_ \\.].*"),
        std::regex(".*[-_ ]rgh[-_ \\.].*"),
    };

    const std::vector<std::regex> occlusionMatchers{
        std::regex(".*[-_ ]ambientocclusion[-_ \\.].*"),
        std::regex(".*[-_ ]occlusion[-_ \\.].*"),
        std::regex(".*[-_ ]ao[-_ \\.].*"),
    };

    const std::vector<std::regex> displacementMatchers{
        std::regex(".*[-_ ]displacement[-_ \\.].*"),
        std::regex(".*[-_ ]disp[-_ \\.].*"),
        std::regex(".*[-_ ]depth[-_ \\.].*"),
    };

    const std::vector<std::regex> opacityMatchers{
        std::regex(".*[-_ ]opacity[-_ \\.].*"),
        std::regex(".*[-_ ]ops[-_ \\.].*"),
        std::regex(".*[-_ ]alpha[-_ \\.].*"),
    };


    std::string resolvePath(
        const std::string& assetsDir,
        const std::string& baseDir,
        const std::string assetPath)
    {
        if (assetPath.empty()) return assetPath;

        const std::string& filePath = util::joinPathExt(
            assetsDir,
            baseDir,
            assetPath, "");

        if (util::fileExists(filePath)) {
            return util::joinPath(baseDir, assetPath);
        }
        return assetPath;
    }
}

namespace loader {
    MaterialLoader::MaterialLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void MaterialLoader::loadMaterialModifiers(
        const loader::DocNode& node,
        MaterialData& data) const
    {
        data.enabled = true;
        data.modifier = true;
        data.material.m_name = "<modifier>";

        loadMaterial(node, data);
    }

    void MaterialLoader::loadMaterials(
        const loader::DocNode& node,
        std::vector<MaterialData>& materials) const
    {
        for (const auto& entry : node.getNodes()) {
            MaterialData& data = materials.emplace_back();
            loadMaterial(entry, data);
            data.materialName = data.material.m_name;
        }
    }

    void MaterialLoader::loadMaterial(
        const loader::DocNode& node,
        MaterialData& data) const
    {
        Material& material = data.material;
        auto& fields = data.fields;

        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::DocNode& v = pair.getNode();

            const auto k = util::toLower(key);

            if (k == "name") {
                material.m_name = readString(v);
            }
            else if (k == "alias") {
                data.aliasName = readString(v);
            }
            else if (k == "modify") {
                data.modifier = readBool(v);
            }
            else if (k == "ns") {
                material.ns = readFloat(v);
                fields.ns = true;
            }
            else if (k == "ka") {
                material.ka = readRGB(v);
                fields.ka = true;
            }
            else if (k == "kd") {
                material.kd = readRGBA(v);
                fields.kd = true;
            }
            else if (k == "ks") {
                material.ks = readRGB(v);
                fields.ks = true;
            }
            else if (k == "ke") {
                material.ke = glm::vec4(readRGB(v), 1.f);
                fields.ke = true;
            }
            else if (k == "ni") {
                material.ni = readFloat(v);
                fields.ni = true;
            }
            else if (k == "d") {
                material.d = readFloat(v);
                fields.d = true;
            }
            else if (k == "illum") {
                material.d = readFloat(v);
                fields.illum = true;
            }
            else if (k == "map_pbr") {
                data.materialPbr = readString(v);
            }
            else if (k == "map_kd") {
                std::string line = readString(v);
                material.addTexPath(
                    TextureType::diffuse,
                    resolveTexturePath(line, true));
            }
            else if (k == "map_ke") {
                std::string line = readString(v);
                material.addTexPath(
                    TextureType::emission,
                    resolveTexturePath(line, true));
            }
            else if (k == "map_ks") {
                std::string line = readString(v);
                material.addTexPath(
                    TextureType::specular,
                    resolveTexturePath(line, true));
            }
            else if (k == "map_bump" || k == "bump") {
                std::string line = readString(v);
                material.addTexPath(
                    TextureType::normal_map,
                    resolveTexturePath(line, true));
            }
            else if (k == "map_bump_strength") {
                material.map_bump_strength = readFloat(v);
                fields.map_bump_strength = true;
            }
            else if (k == "map_dudv") {
                std::string line = readString(v);
                material.addTexPath(
                    TextureType::dudv_map,
                    resolveTexturePath(line, true));
            }
            else if (k == "map_noise") {
                std::string line = readString(v);
                material.addTexPath(
                    TextureType::noise_map,
                    resolveTexturePath(line, true));
            }
            else if (k == "map_roughness") {
                std::string line = readString(v);
                material.addTexPath(
                    TextureType::roughness_map,
                    resolveTexturePath(line, false));
            }
            else if (k == "map_metalness") {
                std::string line = readString(v);
                material.addTexPath(
                    TextureType::metallness_map,
                    resolveTexturePath(line, false));
            }
            else if (k == "map_occlusion") {
                std::string line = readString(v);
                material.addTexPath(
                    TextureType::occlusion_map,
                    resolveTexturePath(line, false));
            }
            else if (k == "map_displacement") {
                std::string line = readString(v);
                material.addTexPath(
                    TextureType::displacement_map,
                    resolveTexturePath(line, false));
            }
            else if (k == "map_opacity") {
                std::string line = readString(v);
                material.addTexPath(
                    TextureType::opacity_map,
                    resolveTexturePath(line, true));
            }
            else if (k == "metal") {
                material.metal = readVec4(v);
                fields.metal = true;
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
            else if (k == "parallax_depth") {
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
            else if (k == "wireframe") {
                material.wireframe = readBool(v);
                fields.wireframe = true;
            }
            else if (k == "program" || k == "shader") {
                material.m_programNames[MaterialProgramType::shader] = readString(v);
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
            else if (k == "geometry_type") {
                material.m_geometryType = readString(v);
            }
            else if (k == "program_definitions") {
                for (const auto& defNode : v.getNodes()) {
                    const auto& defName = defNode.getName();
                    const auto& defValue = readString(defNode.getNode());
                    material.m_programDefinitions[util::toUpper(defName)] = defValue;
                }
            }
            else {
                reportUnknown("material_entry", k, v);
            }
        }

        material.spritesY = material.spriteCount / material.spritesX;
        if (material.spriteCount % material.spritesX != 0) {
            material.spritesY++;
        }
    }

    void MaterialLoader::resolveMaterialPaths(
        const std::string& baseDir,
        MaterialData& data) const
    {
        const auto& assets = Assets::get();
        const auto assetsDir = assets.assetsDir;
        auto& material = data.material;

        for (auto& it : material.modifyTexturePaths()) {
            it.second = resolvePath(assetsDir, baseDir, it.second);
        }
    }

    void MaterialLoader::resolveMaterialPbr(
        const std::string& baseDir,
        MaterialData& data) const
    {
        if (data.materialPbr.empty()) return;
        loadMaterialPbr("", data.materialPbr, data);
        loadMaterialPbr(baseDir, data.materialPbr, data);
    }

    void MaterialLoader::loadMaterialPbr(
        const std::string& baseDir,
        const std::string& pbrName,
        MaterialData& data) const
    {
        if (data.materialPbr.empty()) return;

        const auto& assets = Assets::get();

        const std::string basePath = util::joinPathExt(
            assets.assetsDir,
            baseDir,
            pbrName, "");

        if (!util::dirExists(basePath)) return;

        std::vector<std::filesystem::directory_entry> normalEntries;
        std::vector<std::filesystem::directory_entry> ktxEntries;

        for (const auto& dirEntry : std::filesystem::directory_iterator(basePath)) {
            std::string fileName = dirEntry.path().filename().string();
            std::string matchName{ util::toLower(fileName) };

            if (util::matchAny(texturesMatchers, matchName)) {
                loadMaterialPbr(baseDir, pbrName + "\\" + fileName, data);
                return;
            }

            if (util::matchAny(ignoreMatchers, matchName)) {
                KI_INFO_OUT(fmt::format("IGNORE_PBR_FILE: {} - {}", basePath, fileName));
                continue;
            }

            if (util::matchAny(ktxMatchers, matchName)) {
                ktxEntries.push_back(dirEntry);
            }
            else if (util::matchAny(imageMatchers, matchName)) {
                normalEntries.push_back(dirEntry);
            }
        }

        std::vector<std::string> failedEntries;
        for (const auto& dirEntry : normalEntries) {
            if (!handlePbrEntry(baseDir, pbrName, dirEntry, data)) {
                failedEntries.push_back(dirEntry.path().filename().string());
            }
        }

        if (assets.compressedTexturesEnabled) {
            for (const auto& dirEntry : ktxEntries) {
                if (!handlePbrEntry(baseDir, pbrName, dirEntry, data)) {
                    failedEntries.push_back(dirEntry.path().filename().string());
                }
            }
        }

        if (!failedEntries.empty()) {
            throw std::runtime_error{ fmt::format("UNKNOWN_PBR_FILE: {}", util::join(failedEntries, "\nUNKNOWN_PBR_FILE: ")) };
        }
    }

    bool MaterialLoader::handlePbrEntry(
        const std::string& baseDir,
        const std::string& pbrName,
        const std::filesystem::directory_entry& dirEntry,
        MaterialData& data) const
    {
        const auto& assets = Assets::get();

        Material& material = data.material;
        MaterialField& fields = data.fields;

        const std::string fileName = dirEntry.path().filename().string();
        const std::string assetPath = util::joinPathExt(
            baseDir,
            pbrName,
            fileName, "");

        std::string matchName{ util::toLower(fileName) };

        bool found = false;

        if (!found && util::matchAny(colorMatchers, matchName)) {
            material.addTexPath(
                TextureType::diffuse,
                assetPath);
            found = true;
        }

        if (!found && util::matchAny(emissionMatchers, matchName)) {
            material.addTexPath(
                TextureType::emission,
                assetPath);
            found = true;
        }

        if (!found && util::matchAny(normalMatchers, matchName)) {
            material.addTexPath(
                TextureType::normal_map,
                assetPath);
            found = true;
        }

        if (!found && util::matchAny(metalnessMatchers, matchName)) {
            material.addTexPath(
                TextureType::metallness_map,
                assetPath);
            found = true;
        }

        if (!found && util::matchAny(roughnessMatchers, matchName)) {
            material.addTexPath(
                TextureType::roughness_map,
                assetPath);
            found = true;
        }

        if (!found && util::matchAny(occlusionMatchers, matchName)) {
            material.addTexPath(
                TextureType::occlusion_map,
                assetPath);
            found = true;
        }

        if (!found && util::matchAny(displacementMatchers, matchName)) {
            material.addTexPath(
                TextureType::displacement_map,
                assetPath);
            found = true;
        }

        if (!found && util::matchAny(opacityMatchers, matchName)) {
            material.addTexPath(
                TextureType::opacity_map,
                assetPath);
            found = true;
        }

        return found;
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
        GLint& wrapMode) const
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

        if (f.pattern) m.pattern = mod.pattern;
        if (f.reflection) m.reflection = mod.reflection;
        if (f.refraction) m.refraction = mod.refraction;
        if (f.refractionRatio) m.refractionRatio = mod.refractionRatio;

        if (f.tilingX) m.tilingX = mod.tilingX;
        if (f.tilingY) m.tilingY = mod.tilingY;

        if (f.spriteCount) m.spriteCount = mod.spriteCount;
        if (f.spritesX) m.spritesX = mod.spritesX;
        if (f.spritesX) m.spritesY = mod.spritesY;

        if (f.ns) m.ns = mod.ns;

        if (f.ka) m.ka = mod.ka;
        if (f.kd) m.kd = mod.kd;
        if (f.ks) m.ks = mod.ks;
        if (f.ke) m.ke = mod.ke;

        if (f.map_bump_strength) m.map_bump_strength = mod.map_bump_strength;

        if (f.ni) m.ni = mod.ni;
        if (f.d) m.d = mod.d;
        if (f.illum) m.illum = mod.illum;

        if (f.layers) m.layers = mod.layers;
        if (f.layersDepth) m.layersDepth = mod.layersDepth;
        if (f.parallaxDepth) m.parallaxDepth = mod.parallaxDepth;

        if (f.metal) m.metal = mod.metal;

        if (f.alpha) m.alpha = mod.alpha;
        if (f.blend) m.blend = mod.blend;

        if (f.renderBack) m.renderBack = mod.renderBack;
        if (f.wireframe) m.wireframe = mod.wireframe;

        if (f.gbuffer) m.gbuffer = mod.gbuffer;

        for (const auto& it : mod.getTexturePaths()) {
            m.addTexPath(it.first, it.second);
        }
    }

    void MaterialLoader::resolveMaterial(
        const mesh::MeshType* type,
        Material& material)
    {
        {
            bool useParallax = material.hasBoundTex(TextureType::displacement_map) && material.parallaxDepth > 0;
            if (!useParallax) {
                material.parallaxDepth = 0.f;
            }
        }

        {
            const auto& shaderName = selectProgram(
                MaterialProgramType::shader,
                material.m_programNames);

            if (shaderName.starts_with("g_")) {
                material.gbuffer = true;
            }

            if (material.blend) {
                // NOTE KI alpha MUST BE true if blend
                material.alpha = true;
            }
        }

        material.loadTextures();

        resolveProgram(type, material);
    }

    void MaterialLoader::resolveProgram(
        const mesh::MeshType* type,
        Material& material)
    {
        const bool useDudvTex = material.hasBoundTex(TextureType::dudv_map);
        const bool useDisplacementTex = material.hasBoundTex(TextureType::displacement_map);
        const bool useNormalTex = material.hasBoundTex(TextureType::normal_map);
        const bool useCubeMap = 1.0 - material.reflection - material.refraction < 1.0;
        const bool useNormalPattern = material.pattern > 0;
        const bool useParallax = material.hasBoundTex(TextureType::displacement_map) && material.parallaxDepth > 0;

        const bool useTBN = useNormalTex || useDudvTex || useDisplacementTex;

        const auto& shaderName = selectProgram(
            MaterialProgramType::shader,
            material.m_programNames);

        auto preDepthName = selectProgram(
            MaterialProgramType::pre_depth,
            material.m_programNames,
            SHADER_PRE_DEPTH_PASS);

        const auto& shadowName = selectProgram(
            MaterialProgramType::shadow,
            material.m_programNames);

        const auto& selectionName = selectProgram(
            MaterialProgramType::selection,
            material.m_programNames,
            SHADER_SELECTION);

        const auto& objectIdName = selectProgram(
            MaterialProgramType::object_id,
            material.m_programNames,
            SHADER_OBJECT_ID);

        if (!shaderName.empty()) {
            std::map<std::string, std::string, std::less<>> definitions;
            std::map<std::string, std::string, std::less<>> shadowDefinitions;
            std::map<std::string, std::string, std::less<>> selectionDefinitions;
            std::map<std::string, std::string, std::less<>> idDefinitions;

            for (const auto& [k, v] : material.m_programDefinitions) {
                definitions[k] = v;
            }

            std::map<std::string, std::string, std::less<>> preDepthDefinitions;
            bool usePreDepth = type->m_flags.preDepth;
            bool useBones = type->m_flags.useBones;
            bool useBonesDebug = useBones && type->m_flags.useBonesDebug;

            if (material.alpha) {
                definitions[DEF_USE_ALPHA] = "1";
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
            if (useBones) {
                definitions[DEF_USE_BONES] = "1";
                shadowDefinitions[DEF_USE_BONES] = "1";
                selectionDefinitions[DEF_USE_BONES] = "1";
                idDefinitions[DEF_USE_BONES] = "1";
            }
            if (useBonesDebug) {
                definitions[DEF_USE_BONES_DEBUG] = "1";
            }

            material.m_programs[MaterialProgramType::shader] = ProgramRegistry::get().getProgram(
                shaderName,
                false,
                material.m_geometryType,
                definitions);

            if (!shadowName.empty()) {
                {
                    size_t shadowCount = std::min(
                        std::max(Assets::get().shadowPlanes.size() - 1, static_cast<size_t>(1)),
                        static_cast<size_t>(MAX_SHADOW_MAP_COUNT_ABS));

                    shadowDefinitions[DEF_MAX_SHADOW_MAP_COUNT] = std::to_string(shadowCount);
                }

                material.m_programs[MaterialProgramType::shadow] = ProgramRegistry::get().getProgram(
                    shadowName,
                    false,
                    "",
                    shadowDefinitions);
            }

            if (usePreDepth) {
                material.m_programs[MaterialProgramType::pre_depth] = ProgramRegistry::get().getProgram(
                    preDepthName,
                    false,
                    "",
                    preDepthDefinitions);
            }

            if (!selectionName.empty()) {
                material.m_programs[MaterialProgramType::selection] = ProgramRegistry::get().getProgram(
                    selectionName,
                    false,
                    "",
                    selectionDefinitions);
            }

            if (!objectIdName.empty()) {
                material.m_programs[MaterialProgramType::object_id] = ProgramRegistry::get().getProgram(
                    objectIdName,
                    false,
                    "",
                    idDefinitions);
            }
        }
    }

    std::string MaterialLoader::selectProgram(
        MaterialProgramType type,
        const std::unordered_map<MaterialProgramType, std::string> programs,
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
