#include "DecalLoader.h"

#include "util/util.h"

#include "decal/DecalRegistry.h"

#include "loader/document.h"
#include "loader_util.h"
#include "loader/Loaders.h"

namespace loader {
    DecalLoader::DecalLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void DecalLoader::loadDecals(
        const loader::DocNode& node,
        std::vector<loader::DecalData>& decals,
        Loaders& loaders) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& data = decals.emplace_back();
            loadDecal(entry, data, loaders);
        }
    }

    void DecalLoader::loadDecal(
        const loader::DocNode& node,
        loader::DecalData& data,
        Loaders& loaders) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "lifetime") {
                data.lifetime = readFloat(v);
            }
            else if (k == "scale") {
                data.scale = readFloat(v);
            }
            else if (k == "rotation") {
                data.rotation = readFloat(v);
            }
            else if (k == "sprite_speed") {
                data.spriteSpeed = readFloat(v);
            }
            else if (k == "sprite_base_index") {
                data.spriteBaseIndex = readInt(v);
            }
            else if (k == "rotation_variation") {
                data.rotationVariation = readVec2(v);
            }
            else if (k == "scale_variation") {
                data.scaleVariation = readVec2(v);
            }
            else if (k == "lifetime_variation") {
                data.lifetimeVariation = readVec2(v);
            }
            else if (k == "sprite_speed_variation") {
                data.spriteSpeedVariation = readVec2(v);
            }
            else if (k == "static") {
                data.isStatic = readBool(v);
            }
            else if (k == "material") {
                loaders.m_materialLoader.loadMaterial(v, data.materialData, loaders);
            }
            else {
                reportUnknown("decal_entry", k, v);
            }
        }
    }

    void DecalLoader::createDecals(
        const std::vector<loader::DecalData>& decals,
        Loaders& loaders) const
    {
        for (const auto& data : decals) {
            createDecal(data, loaders);
        }
    }

    void DecalLoader::createDecal(
        const loader::DecalData& data,
        Loaders& loaders) const
    {
        auto mat = data.materialData.material;
        mat.loadTextures();
        mat.registerMaterial();

        decal::DecalDefinition df;
        df.m_sid = SID(data.name);
        df.m_materialIndex = mat.m_registeredIndex;
        df.m_lifetime = data.lifetime;
        df.m_rotation = data.rotation;
        df.m_scale = data.scale;
        df.m_spriteBaseIndex = data.spriteBaseIndex;
        df.m_spriteCount = mat.spriteCount;
        df.m_spriteSpeed = data.spriteSpeed;

        df.m_rotationVariation = data.rotationVariation;
        df.m_scaleVariation = data.scaleVariation;
        df.m_lifetimeVariation = data.lifetimeVariation;
        df.m_spriteSpeedVariation = data.spriteSpeedVariation;

        df.m_static = data.isStatic;

        decal::DecalRegistry::get().addDecal(df);
    }
}
