#include "Loaders.h"

namespace loader {
    Loaders::Loaders(Context ctx)
      : m_rootLoader(ctx),
        m_scriptLoader(ctx),
        m_skyboxLoader(ctx),
        m_volumeLoader(ctx),
        m_cubeMapLoader(ctx),
        m_fontLoader(ctx),
        m_materialLoader(ctx),
        m_customMaterialLoader(ctx),
        m_cameraLoader(ctx),
        m_lightLoader(ctx),
        m_audioLoader(ctx),
        m_controllerLoader(ctx),
        m_generatorLoader(ctx),
        m_particleLoader(ctx),
        m_physicsLoader(ctx),
        m_nodeLoader(ctx),
        m_meshLoader(ctx),
        m_textLoader(ctx),
        m_vertexLoader(ctx),
        m_prefabLoader{ctx}
    {}

    void Loaders::prepare(
        std::shared_ptr<Registry> registry)
    {
        m_fontLoader.setRegistry(registry);
        m_materialLoader.setRegistry(registry);
        m_rootLoader.setRegistry(registry);
        m_scriptLoader.setRegistry(registry);
        m_skyboxLoader.setRegistry(registry);
        m_volumeLoader.setRegistry(registry);
        m_cubeMapLoader.setRegistry(registry);
        m_audioLoader.setRegistry(registry);
        m_generatorLoader.setRegistry(registry);
        m_particleLoader.setRegistry(registry);
        m_physicsLoader.setRegistry(registry);
        m_nodeLoader.setRegistry(registry);
        m_meshLoader.setRegistry(registry);
        m_textLoader.setRegistry(registry);
        m_vertexLoader.setRegistry(registry);
        m_prefabLoader.setRegistry(registry);
    }
}
