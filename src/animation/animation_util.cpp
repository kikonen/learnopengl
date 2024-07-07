#include "animation_util.h"

#include <fstream>

#include "util/Util.h"

#include "ki/yaml.h"

namespace animation
{
    std::unique_ptr<animation::Metadata> loadMetaData(const std::string& meshFilePath)
    {
        const auto& filePath = meshFilePath + ".meta";
        if (!util::fileExists(filePath)) return nullptr;

        std::ifstream fin(filePath);
        YAML::Node yamlRoot = YAML::Load(fin);

        auto metadata = std::make_unique<animation::Metadata>();

        return metadata;
    }
}
