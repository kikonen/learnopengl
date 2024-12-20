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
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "xname" || k == "xxname" || k == "xenabled" || k == "xxenabled") {
                data.explicitEnabled = true;
                data.enabled = false;
            }
            else if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "enabled") {
                data.explicitEnabled = true;
                data.enabled = readBool(v);
            }
            else if (k == "material") {
                loaders.m_materialLoader.loadMaterial(v, data.materialData, loaders);
            }
            else if (k == "seed") {
                data.seed = readInt(v);
            }
            else if (k == "gravity") {
                data.gravity = readVec3(v);
            }
            else if (k == "lifetime") {
                data.lifetime = readFloat(v);
            }
            else if (k == "lifetime_variation") {
                data.lifetimeVariation = readFloat(v);
            }
            else if (k == "area_type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.areaType = particle::AreaType::none;
                }
                else if (type == "point") {
                    data.areaType = particle::AreaType::point;
                }
                else if (type == "disc") {
                    data.areaType = particle::AreaType::disc;
                }
                else if (type == "plane") {
                    data.areaType = particle::AreaType::plane;
                }
                else {
                    reportUnknown("area_type", k, v);
                }
            }
            else if (k == "area_radius") {
                data.areaRadius = readFloat(v);
            }
            else if (k == "area_size") {
                data.areaSize = readVec3(v);
            }
            else if (k == "area_variation") {
                data.areaVariation = readFloat(v);
            }
            else if (k == "dir") {
                data.dir = readVec3(v);
            }
            else if (k == "dir_variation") {
                data.dirVariation = readFloat(v);
            }
            else if (k == "speed") {
                data.speed = readFloat(v);
            }
            else if (k == "speed_variation") {
                data.speedVariation = readFloat(v);
            }
            else if (k == "size") {
                data.size = readFloat(v);
            }
            else if (k == "size_variation") {
                data.sizeVariation = readFloat(v);
            }
            else if (k == "rate") {
                data.rate = readFloat(v);
            }
            else if (k == "rate_variation") {
                data.rateVariation = readFloat(v);
            }
            else if (k == "sprite_base") {
                data.spriteBase = readInt(v);
            }
            else if (k == "sprite_base_variation") {
                data.spriteBaseVariation = readFloat(v);
            }
            else if (k == "sprite_count") {
                data.spriteCount = readInt(v);
            }
            else if (k == "sprite_speed") {
                data.spriteSpeed = readFloat(v);
            }
            else if (k == "sprite_speed_variation") {
                data.spriteSpeedVariation = readFloat(v);
            }
            else {
                reportUnknown("particle_entry", k, v);
            }
        }

        if (!data.explicitEnabled) {
            data.enabled = data.areaType != particle::AreaType::none;
        }
    }

    std::unique_ptr<particle::ParticleGenerator> ParticleLoader::createParticle(
        const ParticleData& data) const
    {
        if (!data.enabled) return nullptr;

        auto generator = std::make_unique<particle::ParticleGenerator>();

        particle::ParticleDefinition df;

        df.m_seed = data.seed;
        df.m_gravity = data.gravity;

        df.m_lifetime = data.lifetime;
        df.m_lifetimeVariation = data.lifetimeVariation;

        df.m_rate = data.rate;
        df.m_rateVariation = data.rateVariation;

        df.m_areaType = data.areaType;
        df.m_areaRadius = data.areaRadius;
        df.m_areaSize = data.areaSize;
        df.m_areaVariation = data.areaVariation;

        df.m_dir = glm::normalize(data.dir);
        df.m_dirVariation = data.dirVariation;

        df.m_speed = data.speed;
        df.m_speedVariation = data.speedVariation;

        df.m_size = data.size;
        df.m_sizeVariation = data.sizeVariation;

        df.m_spriteBase = data.spriteBase;
        df.m_spriteBaseVariation = data.spriteBaseVariation;
        df.m_spriteCount = data.spriteCount;
        if (data.spriteCount < 1) {
            df.m_spriteCount = data.materialData.material.spriteCount;
        }
        df.m_spriteSpeed = data.spriteSpeed;
        df.m_spriteSpeedVariation = data.spriteSpeedVariation;

        generator->setDefinition(df);
        generator->setMaterial(data.materialData.material);
        generator->getMaterial().loadTextures();

        return generator;
    }
}
