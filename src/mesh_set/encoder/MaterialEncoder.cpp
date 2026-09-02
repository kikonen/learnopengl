#include "MaterialEncoder.h"

#include <fmt/format.h>

#include "util/util.h"

#include "material/Material.h"

namespace
{
    void encodeDefinitions(
        YAML::Emitter& out,
        const std::string& key,
        std::map<std::string, std::string> definitions
    )
    {
        if (definitions.empty()) return;

        out << YAML::Key << key;
        out << YAML::Value;
        out << YAML::BeginMap;
        for (const auto& [key, value] : definitions) {
            out << YAML::Key << key;
            out << YAML::Value << value;
        }
        out << YAML::EndMap;
    }

    const std::string getTextureKey(material::TextureType type) noexcept
    {
        switch (type) {
        case material::TextureType::diffuse: return "map_kd";
        case material::TextureType::emission: return "map_ke";
        case material::TextureType::map_normal: return "map_normal";
        case material::TextureType::map_dudv: return "map_dudv";
        case material::TextureType::map_noise: return "map_noise";
        case material::TextureType::map_noise_2: return "map_noise_2";
        case material::TextureType::map_opacity: return "map_opacity";
        case material::TextureType::map_custom_1: return "map_custom_1";
        case material::TextureType::map_mras: return "map_mras";
        case material::TextureType::map_displacement: return "map_displacement";
        }
        return fmt::format("INVALID_TEX_{}", util::as_integer(type));
    }

    const std::string getProgramKey(material::ProgramType type) noexcept
    {
        switch (type) {
        case material::ProgramType::shader: return "program";
        case material::ProgramType::oit: return "oit_program";
        case material::ProgramType::shadow: return "shadow_program";
        case material::ProgramType::pre_depth: return "pre_depth_program";
        case material::ProgramType::selection: return "selection_program";
        case material::ProgramType::object_id: return "id_program";
        case material::ProgramType::normal: return "normal_program";
        }
        return fmt::format("INVALID_PROGRAM_{}", util::as_integer(type));
    }
}

namespace mesh_set::encoder
{
    MaterialEncoder::MaterialEncoder() = default;
    MaterialEncoder::~MaterialEncoder() = default;

    void MaterialEncoder::encode(
        YAML::Emitter& out,
        const util::Ref<Material>& material)
    {
        out << YAML::BeginMap;

        out << YAML::Key << "name";
        out << YAML::Value << material->m_name;

        out << YAML::Key << "base_dir";
        out << YAML::Value << material->m_baseDir;

        out << YAML::Key << "kd";
        out << YAML::Value;
        encodeRGBA(out, material->kd);

        out << YAML::Key << "ke";
        out << YAML::Value;
        encodeRGBA(out, material->ke);

        for (const auto& [type, info] : material->getTextures()) {
            out << YAML::Key << getTextureKey(type);
            out << YAML::Value << info.path;
        }

        out << YAML::Key << "map_bump_strength";
        out << YAML::Value << material->map_bump_strength;

        out << YAML::Key << "mras";
        out << YAML::Value;
        encodeVec4(out, material->mras);

        out << YAML::Key << "invert_roughness";
        out << YAML::Value << material->m_invertRoughness;

        out << YAML::Key << "invert_occlusion";
        out << YAML::Value << material->m_invertOcclusion;

        out << YAML::Key << "invert_metalness";
        out << YAML::Value << material->m_invertMetalness;

        out << YAML::Key << "scale_tiling";
        out << YAML::Value << material->m_scaleTiling;

        out << YAML::Key << "pattern";
        out << YAML::Value << material->pattern;

        out << YAML::Key << "reflection";
        out << YAML::Value << material->reflection;

        out << YAML::Key << "refraction";
        out << YAML::Value << material->refraction;

        out << YAML::Key << "refraction_ratio";
        out << YAML::Value << material->refractionRatio;

        out << YAML::Key << "tiling_x";
        out << YAML::Value << material->tilingX;

        out << YAML::Key << "tiling_y";
        out << YAML::Value << material->tilingY;

        out << YAML::Key << "sprites";
        out << YAML::Value << static_cast<int>(material->spriteCount);

        out << YAML::Key << "sprites_x";
        out << YAML::Value << static_cast<int>(material->spritesX);

        out << YAML::Key << "point_size";
        out << YAML::Value << material->pointSize;

        out << YAML::Key << "layers";
        out << YAML::Value << material->layers;

        out << YAML::Key << "layers_depth";
        out << YAML::Value << material->layersDepth;

        out << YAML::Key << "parallax_depth";
        out << YAML::Value << material->parallaxDepth;

        //out << YAML::Key << "texture_zpec";
        //out << YAML::Value << material->textureSpec;

        out << YAML::Key << "alpha";
        out << YAML::Value << material->alpha;

        out << YAML::Key << "blend";
        out << YAML::Value << material->blend;

        out << YAML::Key << "render_back";
        out << YAML::Value << material->renderBack;

        out << YAML::Key << "line_mode";
        out << YAML::Value << material->lineMode;

        out << YAML::Key << "reverse_front_face";
        out << YAML::Value << material->reverseFrontFace;

        out << YAML::Key << "no_depth";
        out << YAML::Value << material->noDepth;

        out << YAML::Key << "default_programs";
        out << YAML::Value << material->m_defaultPrograms;

        for (const auto& [type, name] : material->m_programNames) {
            out << YAML::Key << getProgramKey(type);
            out << YAML::Value << name;
        }

        out << YAML::Key << "geometry_type";
        out << YAML::Value << material->m_geometryType;

        encodeDefinitions(out, "shared_definitions", material->m_sharedDefinitions);
        encodeDefinitions(out, "program_definitions", material->m_programDefinitions);
        encodeDefinitions(out, "oit_definitions", material->m_oitDefinitions);
        encodeDefinitions(out, "shadow_definitions", material->m_shadowDefinitions);
        encodeDefinitions(out, "selection_definitions", material->m_selectionDefinitions);
        encodeDefinitions(out, "id_definitions", material->m_objectIdDefinitions);
        encodeDefinitions(out, "normal_definitions", material->m_normalDefinitions);
        encodeDefinitions(out, "", material->m_sharedDefinitions);

        out << YAML::Key << "updater_id";
        out << YAML::Value << material->m_updaterId;

        out << YAML::EndMap;
    }
}
