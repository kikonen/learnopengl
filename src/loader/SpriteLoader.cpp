#include "SpriteLoader.h"

#include "util/Util.h"

#include "asset/Sprite.h"
#include "asset/Shape.h"

#include "loader/document.h"

namespace loader {
    SpriteLoader::SpriteLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void SpriteLoader::loadSprites(
        const loader::Node& node,
        std::vector<SpriteData>& sprites)
    {
        for (const auto& entry : node.getNodes()) {
            SpriteData& data = sprites.emplace_back();
            loadSprite(entry, data);
        }
    }

    void SpriteLoader::loadSprite(
        const loader::Node& node,
        SpriteData& data)
    {
        Sprite& sprite = data.sprite;

        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::Node& v = pair.getNode();
            const auto k = util::toLower(key);

            if (k == "name") {
                sprite.m_name = readString(v);
            }
            else if (k == "shapes") {
                loadShapes(v, sprite.m_shapes);
            }
        }
    }

    void SpriteLoader::loadShapes(
        const loader::Node& node,
        std::vector<Shape>& shapes)
    {
        for (const auto& entry : node.getNodes()) {
            Shape& shape = shapes.emplace_back();
            loadShape(entry, shape);
        }
    }

    void SpriteLoader::loadShape(
        const loader::Node& node,
        Shape& shape)
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::Node& v = pair.getNode();
            const auto k = util::toLower(key);

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
