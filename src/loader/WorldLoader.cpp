#include "WorldLoader.h"

#include "asset/Assets.h"

#include "pool/NodeHandle.h"

#include "model/Node.h"
#include "model/NodeType.h"
#include "model/CreateState.h"

#include "event/Dispatcher.h"

#include "engine/Engine.h"

#include "registry/Registry.h"

#include "scene/Scene.h"
#include "scene/World.h"

#include "loader/document.h"
#include "loader_util.h"

#include "Loaders.h"
#include "ScriptData.h"

namespace loader
{
    WorldLoader::WorldLoader(
        const util::Ref<Context>& ctx)
        : BaseLoader(ctx)
    {
    }

    void WorldLoader::loadWorld(
        const loader::DocNode& node,
        WorldData& data) const
    {
        if (node.isNull()) return;

        data.loaded = true;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "time_base") {
                data.timeBaseSecs = parseIso8601ToEpochSecs(readString(v));
            }
            else if (k == "time_scale") {
                data.timeScale = readFloat(v);
            }
            else if (k == "sun_up") {
                data.sunUpSecs = parseClockToSecs(readString(v));
            }
            else if (k == "sun_down") {
                data.sunDownSecs = parseClockToSecs(readString(v));
            }
            else if (k == "sun_angle") {
                data.sunAngle = readFloat(v);
            }
            else if (k == "sun_twilight_angle") {
                data.sunTwilightAngle = readFloat(v);
            }
            else if (k == "day_color") {
                data.dayColor = readRGB(v);
            }
            else if (k == "dusk_color") {
                data.duskColor = readRGB(v);
            }
            else if (k == "night_color") {
                data.nightColor = readRGB(v);
            }
            else {
                reportUnknown("world_entry", k, v);
            }
        }
    }

    void WorldLoader::attachWorld(
        const WorldData& data,
        ScriptSystemData& scriptSystemData,
        Loaders& loaders)
    {
        // no `world:` block -> no World; skybox stays static day (skyBlend 0)
        if (!data.loaded) return;

        auto world = util::Ref<World>::create();
        world->configure(data);

        // World is advanced on WT but owned by the (RT-managed) Scene; publish it
        // to the scene at the RT drain, same pattern as SkyboxLoader::attachSkybox.
        m_registry->invokeLaterRT([registry = m_registry.get(), world]() {
            auto* scene = registry->getEngine().getCurrentScene().get();
            if (scene) {
                scene->setWorld(world);
            }
        });
    }
}
