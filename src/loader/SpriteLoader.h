#pragma once

#include <vector>

#include "BaseLoader.h"

#include "asset/Sprite.h"

namespace loader {
    struct SpriteData {
        Sprite sprite;
    };

    class SpriteLoader : public BaseLoader
    {
    public:
        SpriteLoader(
            Context ctx);

        void loadSprites(
            const YAML::Node& doc);

        void loadSprite(
            const YAML::Node& doc,
            SpriteData& data);

        void loadShapes(
            const YAML::Node& doc,
            std::vector<Shape>& shapes);

        void loadShape(
            const YAML::Node& node,
            Shape& shape);

        Sprite* find(
            std::string_view name);

    private:
        std::vector<SpriteData> m_sprites;
    };
}
