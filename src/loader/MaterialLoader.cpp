#include "MaterialLoader.h"

#include <filesystem>

#include "ki/yaml.h"

#include "util/Util.h"

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
    };

    const std::vector<std::regex> validMatchers{
        std::regex(".*[\\.]hdr"),
        std::regex(".*[\\.]png"),
        std::regex(".*[\\.]jpg"),
    };

    const std::vector<std::regex> colorMatchers{
        std::regex(".*[-_ ]color[-_ \\.].*"),
        std::regex(".*[-_ ]col[-_ \\.].*"),
        std::regex(".*[-_ ]basecolor[-_ \\.].*"),
        std::regex(".*[-_ ]diff[-_ \\.].*"),
    };

    const std::vector<std::regex> emissionMatchers{
        std::regex(".*[-_ ]emission[-_ \\.].*"),
        std::regex(".*[-_ ]emi[-_ \\.].*"),
    };

    const std::vector<std::regex> normalMatchers{
        std::regex(".*[-_ ]normal[-_ \\.].*"),
        std::regex(".*[-_ ]normalgl[-_ \\.].*"),
        std::regex(".*[-_ ]nrm[-_ \\.].*"),
        std::regex(".*[-_ ]nor[-_ \\.].*"),
    };

    const std::vector<std::regex> metalnessMatchers{
        std::regex(".*[-_ ]metalness[-_ \\.].*"),
        std::regex(".*[-_ ]met[-_ \\.].*"),
        std::regex(".*[-_ ]metallic[-_ \\.].*"),
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
    };

    const std::vector<std::regex> opacityMatchers{
        std::regex(".*[-_ ]opacity[-_ \\.].*"),
        std::regex(".*[-_ ]ops[-_ \\.].*"),
    };
}

namespace loader {
    MaterialLoader::MaterialLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
        m_defaultMaterial = Material::createMaterial(BasicMaterial::basic);
    }

    MaterialLoader::~MaterialLoader()
    {
    }

    void MaterialLoader::loadMaterials(
        const YAML::Node& doc) {
        for (const auto& entry : doc["materials"]) {
            MaterialData& data = m_materials.emplace_back();
            loadMaterial(entry, data);
        }
    }

    void MaterialLoader::loadMaterial(
        const YAML::Node& node,
        MaterialData& data)
    {
        Material& material = data.material;
        MaterialField& fields = data.fields;

        for (const auto& pair : node) {
            auto key = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;
            const std::string k = util::toLower(key);

            if (k == "name") {
                material.m_name = readString(v);
            }
            else if (k == "type") {
                std::string type = readString(v);
                if (type == "model") {
                    material.m_type = MaterialType::model;
                    fields.type = true;
                }
                else if (type == "texture") {
                    material.m_type = MaterialType::texture;
                    fields.type = true;
                }
                else if (type == "sprite") {
                    material.m_type = MaterialType::sprite;
                    fields.type = true;
                }
                else {
                    reportUnknown("material_type", k, v);
                }
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
                material.map_kd = resolveTexturePath(line);
                fields.map_kd = true;
            }
            else if (k == "map_ke") {
                std::string line = readString(v);
                material.map_ke = resolveTexturePath(line);
                fields.map_ke = true;
            }
            else if (k == "map_ks") {
                std::string line = readString(v);
                material.map_ks = resolveTexturePath(line);
                fields.map_ks = true;
            }
            else if (k == "map_bump" || k == "bump") {
                std::string line = readString(v);
                material.map_bump = resolveTexturePath(line);
                fields.map_bump = true;
            }
            else if (k == "map_bump_strength") {
                material.map_bump_strength = readFloat(v);
                fields.map_bump_strength = true;
            }
            else if (k == "map_dudv") {
                std::string line = readString(v);
                material.map_dudv = resolveTexturePath(line);
                fields.map_dudv = true;
            }
            else if (k == "map_height") {
                std::string line = readString(v);
                material.map_height = resolveTexturePath(line);
                fields.map_height = true;
            }
            else if (k == "map_noise") {
                std::string line = readString(v);
                material.map_noise = resolveTexturePath(line);
                fields.map_noise = true;
            }
            else if (k == "map_roughness") {
                std::string line = readString(v);
                material.map_roughness = resolveTexturePath(line);
                fields.map_roughness = true;
            }
            else if (k == "map_metalness") {
                std::string line = readString(v);
                material.map_metalness = resolveTexturePath(line);
                fields.map_metalness = true;
            }
            else if (k == "map_occlusion") {
                std::string line = readString(v);
                material.map_occlusion = resolveTexturePath(line);
                fields.map_occlusion = true;
            }
            else if (k == "map_displacement") {
                std::string line = readString(v);
                material.map_displacement = resolveTexturePath(line);
                fields.map_displacement = true;
            }
            else if (k == "map_opacity") {
                std::string line = readString(v);
                material.map_opacity = resolveTexturePath(line);
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
                material.refractionRatio = readRefractionRatio(v);
                fields.refractionRatio = true;
            }
            else if (k == "tiling") {
                material.tilingX = readFloat(v);
                material.tilingY = readFloat(v);
                fields.tilingX = true;
                fields.tilingY = true;
            }
            else if (k == "tiling_x") {
                material.tilingX = readFloat(v);
                fields.tilingX = true;
            }
            else if (k == "tiling_y") {
                material.tilingY = readFloat(v);
                fields.tilingY = true;
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
    }

    void MaterialLoader::loadMaterialPbr(
        const std::string& pbrName,
        MaterialData& data)
    {
        Material& material = data.material;
        MaterialField& fields = data.fields;

        const std::string basePath = util::joinPath(
            m_assets.assetsDir,
            pbrName);

        for (const auto& dirEntry : std::filesystem::directory_iterator(basePath)) {
            std::string fileName = dirEntry.path().filename().string();
            std::string assetPath = util::joinPath(pbrName, fileName);

            std::string matchName{ util::toLower(fileName) };

            if (util::matchAny(texturesMatchers, matchName)) {
                loadMaterialPbr(pbrName + "\\" + fileName, data);
                return;
            }

            if (util::matchAny(ignoreMatchers, matchName)) {
                continue;
            }

            if (!util::matchAny(validMatchers, matchName)) {
                continue;
            }

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

            if (!found) {
                throw std::runtime_error{ fmt::format("UNKNOWN_PBR_FILE: {}", assetPath) };
            }
        }
    }

    void MaterialLoader::loadTextureSpec(
        const YAML::Node& node,
        TextureSpec& textureSpec)
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

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
        const YAML::Node& v,
        GLint& wrapMode)
    {
        const std::string& wrap = readString(v);
        if (wrap == "GL_REPEAT") {
            wrapMode = GL_REPEAT;
        }
        else if (wrap == "GL_CLAMP_TO_EDGE") {
            wrapMode = GL_CLAMP_TO_EDGE;
        }
        else {
            wrapMode = GL_CLAMP_TO_EDGE;
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

        if (f.type) m.m_type = mod.m_type;

        if (f.textureSpec) m.textureSpec = mod.textureSpec;

        if (f.pattern) m.pattern = mod.pattern;
        if (f.reflection) m.reflection = mod.reflection;
        if (f.refraction) m.refraction = mod.refraction;
        if (f.refractionRatio) m.refractionRatio = mod.refractionRatio;

        if (f.tilingX) m.tilingX = mod.tilingX;
        if (f.tilingY) m.tilingY = mod.tilingY;

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
        if (f.map_height) m.map_height = mod.map_height;
        if (f.map_noise) m.map_noise = mod.map_noise;

        if (f.metal) m.metal = mod.metal;
        if (f.map_roughness) m.map_roughness = mod.map_roughness;
        if (f.map_metalness) m.map_metalness = mod.map_metalness;
        if (f.map_occlusion) m.map_occlusion = mod.map_occlusion;
        if (f.map_displacement) m.map_displacement = mod.map_displacement;
        if (f.map_opacity) m.map_opacity = mod.map_opacity;
    }

    Material* MaterialLoader::find(
        std::string_view name)
    {
        const auto& it = std::find_if(
            m_materials.begin(),
            m_materials.end(),
            [&name](MaterialData& m) { return m.material.m_name == name && !m.material.m_default; });
        return it != m_materials.end() ? &(it->material) : nullptr;
    }
}
