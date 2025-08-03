#include "SceneData.h"

#include "material/Material.h"

#include "MetaData.h"
#include "RootData.h"
#include "SkyboxData.h"
#include "ScriptData.h"
#include "NodeTypeData.h"
#include "NodeData.h"
#include "CompositeData.h"
#include "DecalData.h"
#include "ParticleData.h"
#include "MaterialUpdaterData.h"

#include "loader/document.h"

namespace loader
{
    SceneData::SceneData()
        : m_meta{ std::make_unique<MetaData>() },
        m_root{ std::make_unique<RootData>() },
        m_skybox{ std::make_unique<SkyboxData>() },
        m_scriptSystemData{ std::make_unique<ScriptSystemData>() }
    {
        // NOTE KI white causes least unexpectedly tinted results
        m_defaultMaterial = std::make_unique<Material>(Material::createMaterial(BasicMaterial::white));
    }

    SceneData::~SceneData() = default;

    const loader::DocNode* SceneData::findInclude(
        const std::string& filePath)
    {
        const auto& it = std::find_if(
            m_includeFiles.cbegin(),
            m_includeFiles.cend(),
            [&filePath](const auto& e) { return e.first == filePath; });
        return it != m_includeFiles.end() ? &(it->second) : nullptr;
    }
}
