#pragma once

#include <vector>

#include "BaseLoader.h"

#include "SpriteData.h"

namespace loader {
    class SpriteLoader : public BaseLoader
    {
    public:
        SpriteLoader(
            Context ctx);

        void loadSprites(
            const YAML::Node& doc,
            std::vector<SpriteData>& sprites);

        void loadSprite(
            const YAML::Node& doc,
            SpriteData& data);

        void loadShapes(
            const YAML::Node& doc,
            std::vector<Shape>& shapes);

        void loadShape(
            const YAML::Node& node,
            Shape& shape);
    };
}
