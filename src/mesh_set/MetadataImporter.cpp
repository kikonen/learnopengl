#include "MetadataImporter.h"

#include <fstream>

#include <fmt/format.h>

#include "util/Log.h"
#include "util/util.h"
#include "util/assimp_util.h"
#include "util/file.h"

#include "ki/yaml.h"

#include "animation/Clip.h"

#include "animation/MetaData.h"

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

namespace mesh_set
{
    std::unique_ptr<animation::Metadata> MetadataImporter::load(const std::string& meshFilePath)
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

    void MetadataImporter::loadClips(
        const YAML::Node& node,
        std::vector<animation::Clip>& clips)
    {
        for (const auto& entry : node) {
            auto& clip = clips.emplace_back();
            loadClip(entry, clip);
        }
    }

    void MetadataImporter::loadClip(
        const YAML::Node& node,
        animation::Clip& clip)
    {
        float firstFrame = -1.f;
        float lastFrame = -1.f;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const auto& v = pair.second;

            if (k == "name") {
                clip.m_uniqueName = assimp_util::normalizeName(readString(v));
            }
            else if (k == "takeName") {
                clip.m_animationName = assimp_util::normalizeName(readString(v));
            }
            else if (k == "firstFrame") {
                // NOTE KI try to avoid errors due to weird cases like this
                //firstFrame: 1969.9999
                //lastFrame : 2020.0001
                firstFrame = readFloat(v);
                clip.m_firstFrame = static_cast<uint16_t>(round(firstFrame));
            }
            else if (k == "lastFrame") {
                lastFrame = readFloat(v);
                clip.m_lastFrame = static_cast<uint16_t>(round(lastFrame));
            }
            else if (k == "loop") {
                clip.m_loop = readInt(v) == 1;
            }
        }

        KI_DEBUG(fmt::format(
            "CLIP: {}, clipFirstFrame={}, clipLastFrame={}, firstFrame={}, lastFrame={}",
            clip.m_uniqueName, clip.m_firstFrame, clip.m_lastFrame, firstFrame, lastFrame));

        clip.m_single = false;
    }
}
