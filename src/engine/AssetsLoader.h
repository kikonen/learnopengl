#pragma once

#include <string>
#include <map>

#include "ki/yaml.h"

#include "asset/Assets.h"

class AssetsLoader final
{
public:
    AssetsLoader(
        std::string_view filename);
    ~AssetsLoader();

    Assets load();

private:
    void resolveDirs(
        Assets& data);

    void loadAssets(
        const YAML::Node& node,
        Assets& data);

    void loadLayers(
        const YAML::Node& node,
        std::vector<LayerInfo>& layers);

    void loadLayer(
        const YAML::Node& node,
        LayerInfo& layer);

    ViewportEffect readViewportEffect(
        const std::string& key,
        const YAML::Node& node) const;

private:
    const std::string m_filename;
};

