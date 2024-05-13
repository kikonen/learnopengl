#include "MaterialLoader.h"

#include <filesystem>

#include "asset/Assets.h"

#include "util/Util.h"

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
}

namespace loader {
    MaterialLoader::MaterialLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void MaterialLoader::loadMaterialModifiers(
        const loader::Node& node,
        MaterialData& data) const
    {
        data.enabled = true;
        data.material.m_name = "<modifier>";

        loadMaterial(node, data);
    }

    void MaterialLoader::loadMaterials(
        const loader::Node& node,
        std::vector<MaterialData>& materials) const
    {
        for (const auto& entry : node.getNodes()) {
            MaterialData& data = materials.emplace_back();
            loadMaterial(entry, data);
            data.materialName = data.material.m_name;
        }
    }

    void MaterialLoader::loadMaterial(
        const loader::Node& node,
        MaterialData& data) const
    {
        Material& material = data.material;
        auto& fields = data.fields;

        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::Node& v = pair.getNode();

            const auto k = util::toLower(key);

            if (k == "name") {
                material.m_name = readString(v);
            }
            else if (k == "alias") {
                data.aliasName = readString(v);
            }
            else if (k == "modify") {
                data.modify = readBool(v);
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
                std::string line = readString(v);
                loadMaterialPbr(
                    line,
                    data);
            }
            else if (k == "map_kd") {
                std::string line = readString(v);
                material.map_kd = resolveTexturePath(line, true);
                fields.map_kd = true;
            }
            else if (k == "map_ke") {
                std::string line = readString(v);
                material.map_ke = resolveTexturePath(line, true);
                fields.map_ke = true;
            }
            else if (k == "map_ks") {
                std::string line = readString(v);
                material.map_ks = resolveTexturePath(line, true);
                fields.map_ks = true;
            }
            else if (k == "map_bump" || k == "bump") {
                std::string line = readString(v);
                material.map_bump = resolveTexturePath(line, true);
                fields.map_bump = true;
            }
            else if (k == "map_bump_strength") {
                material.map_bump_strength = readFloat(v);
                fields.map_bump_strength = true;
            }
            else if (k == "map_dudv") {
                std::string line = readString(v);
                material.map_dudv = resolveTexturePath(line, true);
                fields.map_dudv = true;
            }
            else if (k == "map_noise") {
                std::string line = readString(v);
                material.map_noise = resolveTexturePath(line, true);
                fields.map_noise = true;
            }
            else if (k == "map_roughness") {
                std::string line = readString(v);
                material.map_roughness = resolveTexturePath(line, false);
                fields.map_roughness = true;
            }
            else if (k == "map_metalness") {
                std::string line = readString(v);
                material.map_metalness = resolveTexturePath(line, false);
                fields.map_metalness = true;
            }
            else if (k == "map_occlusion") {
                std::string line = readString(v);
                material.map_occlusion = resolveTexturePath(line, false);
                fields.map_occlusion = true;
            }
            else if (k == "map_displacement") {
                std::string line = readString(v);
                material.map_displacement = resolveTexturePath(line, false);
                fields.map_displacement = true;
            }
            else if (k == "map_opacity") {
                std::string line = readString(v);
                material.map_opacity = resolveTexturePath(line, true);
                fields.map_opacity = true;
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
            else {
                reportUnknown("material_entry", k, v);
            }
        }

        material.spritesY = material.spriteCount / material.spritesX;
        if (material.spriteCount % material.spritesX != 0) {
            material.spritesY++;
        }
    }

    void MaterialLoader::loadMaterialPbr(
        const std::string& pbrName,
        MaterialData& data) const
    {
        const auto& assets = Assets::get();

        const std::string basePath = util::joinPath(
            assets.assetsDir,
            pbrName);

        std::vector<std::filesystem::directory_entry> normalEntries;
        std::vector<std::filesystem::directory_entry> ktxEntries;

        for (const auto& dirEntry : std::filesystem::directory_iterator(basePath)) {
            std::string fileName = dirEntry.path().filename().string();
            std::string matchName{ util::toLower(fileName) };

            if (util::matchAny(texturesMatchers, matchName)) {
                loadMaterialPbr(pbrName + "\\" + fileName, data);
                return;
            }

            if (util::matchAny(ignoreMatchers, matchName)) {
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
            if (!handlePbrEntry(pbrName, dirEntry, data)) {
                failedEntries.push_back(dirEntry.path().filename().string());
            }
        }

        if (assets.compressedTexturesEnabled) {
            for (const auto& dirEntry : ktxEntries) {
                if (!handlePbrEntry(pbrName, dirEntry, data)) {
                    failedEntries.push_back(dirEntry.path().filename().string());
                }
            }
        }

        if (!failedEntries.empty()) {
            throw std::runtime_error{ fmt::format("UNKNOWN_PBR_FILE: {}", util::join(failedEntries, "\nUNKNOWN_PBR_FILE: ")) };
        }
    }

    bool MaterialLoader::handlePbrEntry(
        const std::string& pbrName,
        const std::filesystem::directory_entry& dirEntry,
        MaterialData& data) const
    {
        const auto& assets = Assets::get();

        Material& material = data.material;
        MaterialField& fields = data.fields;

        const std::string fileName = dirEntry.path().filename().string();
        const std::string assetPath = util::joinPath(pbrName, fileName);

        std::string matchName{ util::toLower(fileName) };

        bool found = false;

        if (!found && util::matchAny(colorMatchers, matchName)) {
            fields.map_kd = true;
            material.map_kd = assetPath;
            found = true;
        }

        if (!found && util::matchAny(emissionMatchers, matchName)) {
            fields.map_ke = true;
            material.map_ke = assetPath;
            found = true;
        }

        if (!found && util::matchAny(normalMatchers, matchName)) {
            fields.map_bump = true;
            material.map_bump = assetPath;
            found = true;
        }

        if (!found && util::matchAny(metalnessMatchers, matchName)) {
            fields.map_metalness = true;
            material.map_metalness = assetPath;
            found = true;
        }

        if (!found && util::matchAny(roughnessMatchers, matchName)) {
            fields.map_roughness = true;
            material.map_roughness = assetPath;
            found = true;
        }

        if (!found && util::matchAny(occlusionMatchers, matchName)) {
            fields.map_occlusion = true;
            material.map_occlusion = assetPath;
            found = true;
        }

        if (!found && util::matchAny(displacementMatchers, matchName)) {
            fields.map_displacement = true;
            material.map_displacement = assetPath;
            found = true;
        }

        if (!found && util::matchAny(opacityMatchers, matchName)) {
            fields.map_opacity = true;
            material.map_opacity = assetPath;
            found = true;
        }

        return found;
    }

    void MaterialLoader::loadTextureSpec(
        const loader::Node& node,
        TextureSpec& textureSpec) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::Node& v = pair.getNode();

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
        const loader::Node& v,
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
        if (f.map_kd) m.map_kd = mod.map_kd;

        if (f.ks) m.ks = mod.ks;
        if (f.map_ks) m.map_ks = mod.map_ks;
        if (f.ke) m.ke = mod.ke;
        if (f.map_ke) m.map_ke = mod.map_ke;
        if (f.map_bump) m.map_bump = mod.map_bump;
        if (f.map_bump_strength) m.map_bump_strength = mod.map_bump_strength;
        if (f.ni) m.ni = mod.ni;
        if (f.d) m.d = mod.d;
        if (f.illum) m.illum = mod.illum;

        if (f.layers) m.layers = mod.layers;
        if (f.layersDepth) m.layersDepth = mod.layersDepth;
        if (f.parallaxDepth) m.parallaxDepth = mod.parallaxDepth;

        if (f.map_dudv) m.map_dudv = mod.map_dudv;
        if (f.map_noise) m.map_noise = mod.map_noise;

        if (f.metal) m.metal = mod.metal;
        if (f.map_roughness) m.map_roughness = mod.map_roughness;
        if (f.map_metalness) m.map_metalness = mod.map_metalness;
        if (f.map_occlusion) m.map_occlusion = mod.map_occlusion;
        if (f.map_displacement) m.map_displacement = mod.map_displacement;
        if (f.map_opacity) m.map_opacity = mod.map_opacity;
    }
}
