#include "ChannelTextureLoader.h"

#include <map>

#include <fmt/format.h>

#include "util/Log.h"
#include "util/Util.h"

#include "loader_util.h"

namespace {
    const std::string CHANNEL_NONE{ "X" };
    const std::string CHANNEL_RED{ "R" };
    const std::string CHANNEL_GREEN{ "G" };
    const std::string CHANNEL_BLUE{ "B" };
    const std::string CHANNEL_ALPHA{ "A" };

    std::map<std::string, ChannelPart::Channel> g_channels{
        { CHANNEL_NONE, ChannelPart::Channel::none },
        { CHANNEL_RED, ChannelPart::Channel::red },
        { CHANNEL_GREEN, ChannelPart::Channel::green},
        { CHANNEL_BLUE, ChannelPart::Channel::blue},
        { CHANNEL_ALPHA, ChannelPart::Channel::alpha},
    };

    std::map<std::string, TextureType> g_textureTypes{
        { "diffuse", TextureType::diffuse},
        { "emission", TextureType::emission },
        { "specular", TextureType::specular },
        { "normal_map", TextureType::normal_map },
        { "dudv_map", TextureType::dudv_map },
        { "noise_map", TextureType::noise_map },
        { "metallness_map", TextureType::metallness_map },
        { "roughness_map", TextureType::roughness_map },
        { "occlusion_map", TextureType::occlusion_map },
        { "displacement_map", TextureType::displacement_map },
        { "opacity_map", TextureType::opacity_map },
        { "channel_part_1_map", TextureType::channel_part_1_map },
        { "channel_part_2_map", TextureType::channel_part_2_map },
        { "channel_part_3_map", TextureType::channel_part_3_map },
        { "channel_part_4_map", TextureType::channel_part_4_map },
        { "metal_channel_map", TextureType::metal_channel_map },
    };

    std::map<std::string, TextureType>& getTextureTypeMapping() {
        return g_textureTypes;
    }

    std::map<std::string, ChannelPart::Channel>& getChannelMapping() {
        return g_channels;
    }

    TextureType resolveTextureType(const std::string& p, TextureType defaultType) {
        const auto& mapping = getTextureTypeMapping();
        const auto& it = mapping.find(p);
        if (it != mapping.end()) return it->second;
        KI_WARN_OUT(fmt::format("LOADER_CHANNEL: missing_type={}", p));
    }

    ChannelPart::Channel resolveChannel(const std::string& p) {
        const auto& mapping = getChannelMapping();
        const auto& it = mapping.find(p);
        if (it != mapping.end()) return it->second;
        return ChannelPart::Channel::none;
    }

    std::vector<ChannelPart::Channel> readChannelVector(
        const loader::DocNode& node)
    {
        std::vector<ChannelPart::Channel> vec;
        vec.reserve(4);

        std::string str = readString(node);

        for (int i = 0; i < str.size(); i++) {
            std::string p{ str[i] };
            vec.push_back(resolveChannel(util::toUpper(p)));
        }

        while (vec.size() < 4) {
            vec.push_back(ChannelPart::Channel::none);
        }

        return vec;
    }

    void readChannelMapping(
        const loader::DocNode& node,
        ChannelPart& part)
    {
        auto vec = readChannelVector(node);

        for (int i = 0; i < 4; i++) {
            part.m_mapping[i] = vec[i];
        }
    }
}

namespace loader {
    void ChannelTextureLoader::loadParts(
        const loader::DocNode& node,
        Material& material) const
    {
        auto& parts = material.map_channelParts;

        static TextureType types[4] = {
            TextureType::channel_part_1_map,
            TextureType::channel_part_2_map,
            TextureType::channel_part_3_map,
            TextureType::channel_part_4_map,
        };

        int idx = 0;
        for (const auto& node : node.getNodes()) {
            // NOTE KI max 4 parts can exist (i.e. RGBA == 4)
            if (idx >= 4) break;
            auto& part = parts.emplace_back();
            loadPart(node, material, types[idx++], part);
        }
    }

    void ChannelTextureLoader::loadPart(
        const loader::DocNode& node,
        Material& material,
        TextureType type,
        ChannelPart& part) const
    {
        part.m_type = type;

        bool enabled = true;
        std::string path;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "path" || k == "xpath") {
                path = readString(v);
                enabled = k == "path";
            }
            else if (k == "type") {
                part.m_type = resolveTextureType(readString(v), type);
            }
            else if (k == "channel") {
                readChannelMapping(v, part);
            }
            else {
                reportUnknown("channel_part_entry", k, v);
            }
        }

        if (enabled) {
            if (!path.empty()) {
                material.addTexPath(part.m_type, path);
            }
        }
        else {
            part.m_type = TextureType::none;
            for (auto& map : part.m_mapping) {
                map = ChannelPart::Channel::none;
            }
        }
    }
}
