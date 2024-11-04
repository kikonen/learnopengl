#pragma once

#include "Context.h"

#include "BaseLoader.h"
#include "RootLoader.h"
#include "ScriptLoader.h"
#include "SkyboxLoader.h"
#include "FontLoader.h"
#include "MaterialLoader.h"
#include "MaterialUpdaterLoader.h"
#include "CustomMaterialLoader.h"
#include "CameraLoader.h"
#include "LightLoader.h"
#include "ControllerLoader.h"
#include "AudioLoader.h"
#include "GeneratorLoader.h"
#include "ParticleLoader.h"
#include "DecalLoader.h"
#include "PhysicsLoader.h"
#include "PrefabLoader.h"
#include "NodeLoader.h"
#include "MeshLoader.h"
#include "TextLoader.h"
#include "VertexLoader.h"

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

        NodeLoader m_nodeLoader;
        MeshLoader m_meshLoader;
        TextLoader m_textLoader;

        VertexLoader m_vertexLoader;

        FontLoader m_fontLoader;
        MaterialLoader m_materialLoader;
        MaterialUpdaterLoader m_materialUpdaterLoader;
        CustomMaterialLoader m_customMaterialLoader;

        CameraLoader m_cameraLoader;
        LightLoader m_lightLoader;
        ControllerLoader m_controllerLoader;
        AudioLoader m_audioLoader;
        GeneratorLoader m_generatorLoader;
        ParticleLoader m_particleLoader;
        DecalLoader m_decalLoader;
        PhysicsLoader m_physicsLoader;

        PrefabLoader m_prefabLoader;
    };
}
