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
            const YAML::Node& node,
            std::vector<SpriteData>& sprites);

        void loadSprite(
            const YAML::Node& node,
            SpriteData& data);

        void loadShapes(
            const YAML::Node& node  ,
            std::vector<Shape>& shapes);

        void loadShape(
            const YAML::Node& node,
            Shape& shape);
    };
}
