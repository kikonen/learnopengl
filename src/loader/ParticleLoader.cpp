#include "ParticleLoader.h"

#include "util/util.h"

#include "material/Material.h"

#include "particle/ParticleGenerator.h"

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
                loaders.m_materialLoader.loadMaterial(v, data.materialData, loaders);
            }
            else if (k == "dir") {
                data.dir = readVec3(v);
            }
            else if (k == "dir_variation") {
                data.dirVariation = readFloat(v);
            }
            else if (k == "lifetime") {
                data.lifetime = readFloat(v);
            }
            else if (k == "radius") {
                data.radius = readFloat(v);
            }
            else if (k == "velocity") {
                data.velocity = readFloat(v);
            }
            else if (k == "velocity_variation") {
                data.velocityVariation = readFloat(v);
            }
            else if (k == "size") {
                data.size = readFloat(v);
            }
            else if (k == "size_variation") {
                data.velocityVariation = readFloat(v);
            }
            else if (k == "rate") {
                data.rate = readFloat(v);
            }
            else if (k == "rate_variation") {
                data.rateVariation = readFloat(v);
            }
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

        particle::ParticleDefinition def;

        def.dir = data.dir;
        def.dirVariation = data.dirVariation;

        def.lifetime = data.lifetime;
        def.radius = data.radius;

        def.velocity = data.velocity;
        def.velocityVariation = data.velocityVariation;

        def.size = data.size;
        def.sizeVariation = data.sizeVariation;

        def.rate = data.rate;

        generator->setDefinition(def);
        generator->setMaterial(data.materialData.material);
        generator->getMaterial().loadTextures();

        return generator;
    }
}
