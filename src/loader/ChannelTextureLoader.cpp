#include "ChannelTextureLoader.h"

#include <map>

#include "util/Util.h"

#include "loader_util.h"

namespace {
    const std::string CHANNEL_NONE{ "X" };
    const std::string CHANNEL_RED{ "R" };
    const std::string CHANNEL_GREEN{ "G" };
    const std::string CHANNEL_BLUE{ "B" };
    const std::string CHANNEL_ALPHA{ "A" };

    std::map<std::string, ChannelPart::Channel> m_channels;

    std::map<std::string, ChannelPart::Channel>& getChannelMapping() {
        if (m_channels.empty()) {
            m_channels[CHANNEL_NONE] = ChannelPart::Channel::none;
            m_channels[CHANNEL_RED] = ChannelPart::Channel::red;
            m_channels[CHANNEL_GREEN] = ChannelPart::Channel::green;
            m_channels[CHANNEL_BLUE] = ChannelPart::Channel::blue;
            m_channels[CHANNEL_ALPHA] = ChannelPart::Channel::alpha;
        }
        return m_channels;
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

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "path") {
                material.addTexPath(type, readString(v));
            }
            else if (k == "channel") {
                readChannelMapping(v, part);
            }
        }
    }
}
