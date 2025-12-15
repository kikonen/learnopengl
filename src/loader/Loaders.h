#pragma once

#include "BaseLoader.h"
#include "RootLoader.h"
#include "IncludeLoader.h"
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
#include "NodeTypeLoader.h"
#include "NodeLoader.h"
#include "CompositeLoader.h"
#include "MeshLoader.h"
#include "TextLoader.h"
#include "VertexLoader.h"

class Registry;

namespace loader {
    class Loaders {
    public:
        Loaders(const std::shared_ptr<Context>& ctx);

        void prepare(
            Registry* registry);

    public:
        RootLoader m_rootLoader;

        ScriptLoader m_scriptLoader;

        SkyboxLoader m_skyboxLoader;

        IncludeLoader m_includeLoader;

        NodeTypeLoader m_nodeTypeLoader;
        NodeLoader m_nodeLoader;
        CompositeLoader m_compositeLoader;

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
