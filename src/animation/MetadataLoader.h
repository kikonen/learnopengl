#pragma once

#include <string>
#include <memory>
#include <vector>

namespace YAML {
    class Node;
}

namespace animation
{
    struct Metadata;
    struct Clip;

    class MetadataLoader
    {
    public:
        std::unique_ptr<animation::Metadata> load(const std::string& meshFilePath);

    private:
        void loadClips(
            const YAML::Node& node,
            std::vector<animation::Clip>& clips);

        void loadClip(
            const YAML::Node& node,
            animation::Clip& clip);
    };
}
