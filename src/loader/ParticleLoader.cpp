#include "ParticleLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "asset/Material.h"

#include "particle/ParticleGenerator.h"

#include "registry/MaterialRegistry.h"

namespace loader
{
    ParticleLoader::ParticleLoader(
        Context ctx)
        : BaseLoader(ctx)
    {}

    void ParticleLoader::loadParticle(
        const YAML::Node& node,
        ParticleData& data) const
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "name") {
                data.name = readString(v);
            }
            //else if (k == "path") {
            //    data.path = readString(v);
            //}
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
        const ParticleData& data,
        const std::vector<MaterialData>& materials) const
    {
        if (!data.enabled) return nullptr;

        auto generator = std::make_unique<particle::ParticleGenerator>();

        auto* material = findMaterial(data.materialName, materials);
        if (material) {
            generator->setMaterial(*material);
        }

        return generator;
    }
}
