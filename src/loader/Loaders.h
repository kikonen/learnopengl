#pragma once

#include "Context.h"

#include "BaseLoader.h"
#include "RootLoader.h"
#include "ScriptLoader.h"
#include "SkyboxLoader.h"
#include "VolumeLoader.h"
#include "CubeMapLoader.h"
#include "FontLoader.h"
#include "MaterialLoader.h"
#include "CustomMaterialLoader.h"
#include "CameraLoader.h"
#include "LightLoader.h"
#include "ControllerLoader.h"
#include "AudioLoader.h"
#include "GeneratorLoader.h"
#include "ParticleLoader.h"
#include "PhysicsLoader.h"
#include "PrefabLoader.h"
#include "EntityLoader.h"

class Registry;

namespace loader {
    class Loaders {
    public:
        Loaders(Context ctx);

        void prepare(
            std::shared_ptr<Registry> registry);

    public:
        RootLoader m_rootLoader;

        ScriptLoader m_scriptLoader;

        SkyboxLoader m_skyboxLoader;
        VolumeLoader m_volumeLoader;
        CubeMapLoader m_cubeMapLoader;

        EntityLoader m_entityLoader;

        FontLoader m_fontLoader;
        MaterialLoader m_materialLoader;
        CustomMaterialLoader m_customMaterialLoader;

        CameraLoader m_cameraLoader;
        LightLoader m_lightLoader;
        ControllerLoader m_controllerLoader;
        AudioLoader m_audioLoader;
        GeneratorLoader m_generatorLoader;
        ParticleLoader m_particleLoader;
        PhysicsLoader m_physicsLoader;

        PrefabLoader m_prefabLoader;
    };
}
