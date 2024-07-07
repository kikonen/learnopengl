#include "MetadataLoader.h"

#include <fstream>

#include <fmt/format.h>

#include "util/Log.h"
#include "util/Util.h"

#include "ki/yaml.h"

#include "Metadata.h"
#include "Clip.h"

namespace {
    std::string renderNode(
        const YAML::Node& v)
    {
        std::stringstream ss;
        ss << v;
        return ss.str();
    }

    std::string readString(const YAML::Node& node)
    {
        return node.as<std::string>();
    }

    bool readBool(const YAML::Node& node)
    {
        if (!util::isBool(readString(node))) {
            KI_WARN(fmt::format("invalid bool={}", renderNode(node)));
            return false;
        }

        return node.as<bool>();
    }

    int readInt(const YAML::Node& node)
    {
        if (!util::isInt(readString(node))) {
            KI_WARN(fmt::format("invalid int{}", renderNode(node)));
            return 0;
        }

        return node.as<int>();
    }

    float readFloat(const YAML::Node& node)
    {
        if (!util::isFloat(readString(node))) {
            KI_WARN(fmt::format("invalid float {}", renderNode(node)));
            return 0.f;
        }

        return node.as<float>();
    }
}

namespace animation
{
    std::unique_ptr<animation::Metadata> MetadataLoader::load(const std::string& meshFilePath)
    {
        const auto& filePath = meshFilePath + ".meta";
        if (!util::fileExists(filePath)) return nullptr;

        std::ifstream fin(filePath);
        YAML::Node yamlRoot = YAML::Load(fin);

        auto metadata = std::make_unique<animation::Metadata>();

        const auto& modelNode = yamlRoot["ModelImporter"];
        const auto& animationsNode  = modelNode["animations"];

        //for (const auto& pair : animationsNode) {
        //    const std::string& k = pair.first.as<std::string>();
        //    const auto& v = pair.second;

        //    KI_INFO_OUT(fmt::format("{}={}", k, renderNode(v)));
        //}

        loadClips(animationsNode["clipAnimations"], metadata->m_clips);

        return metadata;
    }

    void MetadataLoader::loadClips(
        const YAML::Node& node,
        std::vector<animation::Clip>& clips)
    {
        for (const auto& entry : node) {
            auto& clip = clips.emplace_back();
            loadClip(entry, clip);
        }
    }

    void MetadataLoader::loadClip(
        const YAML::Node& node,
        animation::Clip& clip)
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const auto& v = pair.second;

            if (k == "name") {
                clip.m_name = readString(v);
            }
            else if (k == "takeName") {
                clip.m_animationName = readString(v);
            }
            else if (k == "firstFrame") {
                clip.m_firstFrame = readInt(v);
            }
            else if (k == "lastFrame") {
                clip.m_lastFrame = readInt(v);
            }
        }
    }
}
