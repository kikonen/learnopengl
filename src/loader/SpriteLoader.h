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
            const loader::Node& node,
            std::vector<SpriteData>& sprites);

        void loadSprite(
            const loader::Node& node,
            SpriteData& data);

        void loadShapes(
            const loader::Node& node  ,
            std::vector<Shape>& shapes);

        void loadShape(
            const loader::Node& node,
            Shape& shape);
    };
}
