#include "Loaders.h"

namespace loader {
    Loaders::Loaders(std::shared_ptr<Context> ctx)
      : m_rootLoader(ctx),
        m_includeLoader(ctx),
        m_scriptLoader(ctx),
        m_skyboxLoader(ctx),
        m_fontLoader(ctx),
        m_materialLoader(ctx),
        m_materialUpdaterLoader(ctx),
        m_customMaterialLoader(ctx),
        m_cameraLoader(ctx),
        m_lightLoader(ctx),
        m_audioLoader(ctx),
        m_controllerLoader(ctx),
        m_generatorLoader(ctx),
        m_particleLoader(ctx),
        m_decalLoader(ctx),
        m_physicsLoader(ctx),
        m_nodeTypeLoader(ctx),
        m_nodeLoader(ctx),
        m_compositeLoader(ctx),
        m_meshLoader(ctx),
        m_textLoader(ctx),
        m_vertexLoader(ctx),
        m_prefabLoader{ctx}
    {}

    void Loaders::prepare(
        std::shared_ptr<Registry> registry)
    {
        m_includeLoader.setRegistry(registry);
        m_fontLoader.setRegistry(registry);
        m_materialLoader.setRegistry(registry);
        m_materialUpdaterLoader.setRegistry(registry);
        m_customMaterialLoader.setRegistry(registry);
        m_rootLoader.setRegistry(registry);
        m_scriptLoader.setRegistry(registry);
        m_skyboxLoader.setRegistry(registry);
        m_audioLoader.setRegistry(registry);
        m_generatorLoader.setRegistry(registry);
        m_particleLoader.setRegistry(registry);
        m_decalLoader.setRegistry(registry);
        m_physicsLoader.setRegistry(registry);
        m_nodeTypeLoader.setRegistry(registry);
        m_nodeLoader.setRegistry(registry);
        m_compositeLoader.setRegistry(registry);
        m_meshLoader.setRegistry(registry);
        m_textLoader.setRegistry(registry);
        m_vertexLoader.setRegistry(registry);
        m_prefabLoader.setRegistry(registry);
    }
}
