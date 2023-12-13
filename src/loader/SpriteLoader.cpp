#include "SpriteLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "asset/Sprite.h"
#include "asset/Shape.h"

namespace loader {
    SpriteLoader::SpriteLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void SpriteLoader::loadSprites(
        const YAML::Node& node,
        std::vector<SpriteData>& sprites)
    {
        for (const auto& entry : node) {
            SpriteData& data = sprites.emplace_back();
            loadSprite(entry, data);
        }
    }

    void SpriteLoader::loadSprite(
        const YAML::Node& node,
        SpriteData& data)
    {
        Sprite& sprite = data.sprite;

        for (const auto& pair : node) {
            auto key = pair.first.as<std::string>();
            const std::string k = util::toLower(key);
            const YAML::Node& v = pair.second;

            if (k == "name") {
                sprite.m_name = readString(v);
            }
            else if (k == "shapes") {
                loadShapes(v, sprite.m_shapes);
            }
        }
    }

    void SpriteLoader::loadShapes(
        const YAML::Node& node,
        std::vector<Shape>& shapes)
    {
        for (const auto& entry : node) {
            Shape& shape = shapes.emplace_back();
            loadShape(entry, shape);
        }
    }

    void SpriteLoader::loadShape(
        const YAML::Node& node,
        Shape& shape)
    {
        for (const auto& pair : node) {
            auto key = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;
            const std::string k = util::toLower(key);

            if (k == "rot" || k == "rotation") {
                shape.m_rotation = readFloat(v);
            }
            //else if (k == "material") {
            //    loadMaterial(
            //        v,
            //        shape.m_materialFields,
            //        shape.m_material);
            //}
            else {
                reportUnknown("shape_entry", k, v);
            }
        }
    }
}
