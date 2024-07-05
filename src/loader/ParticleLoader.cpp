#include "ParticleLoader.h"

#include "util/Util.h"

#include "asset/Material.h"

#include "particle/ParticleGenerator.h"

#include "registry/MaterialRegistry.h"

#include "loader/document.h"
#include "loader_util.h"

#include "loader/Loaders.h"

namespace loader
{
    ParticleLoader::ParticleLoader(
        Context ctx)
        : BaseLoader(ctx)
    {}

    void ParticleLoader::loadParticle(
        const loader::DocNode& node,
        ParticleData& data,
        Loaders& loaders) const
    {
        data.enabled = true;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "xname" || k == "xxname" || k == "xenabled" || k == "xxenabled") {
                data.enabled = false;
            }
            else if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "material") {
                loaders.m_materialLoader.loadMaterial(v, data.materialData);
            }
            //else if (k == "size") {
            //    data.size = readFloat(v);
            //}
            //else if (k == "atlas_size") {
            //    data.atlasSize = readVec2(v);
            //}
            else {
                reportUnknown("particle_entry", k, v);
            }
        }
    }

    std::unique_ptr<particle::ParticleGenerator> ParticleLoader::createParticle(
        const ParticleData& data) const
    {
        if (!data.enabled) return nullptr;

        auto generator = std::make_unique<particle::ParticleGenerator>();

        generator->setMaterial(data.materialData.material);
        generator->getMaterial().loadTextures();

        return generator;
    }
}
