#include "ModelLoader.h"

#include "util/Util.h"

#include "mesh/MeshSet.h"


namespace mesh {
    ModelLoader::ModelLoader(
        std::shared_ptr<std::atomic<bool>> alive)
        : m_alive(alive)
    {
    }

    ModelLoader::~ModelLoader()
    {
    }

    bool ModelLoader::load(
        mesh::MeshSet& meshSet,
        Material* defaultMaterial,
        bool forceDefaultMaterial)
    {
        if (defaultMaterial) {
            m_defaultMaterial = *defaultMaterial;
        }
        else {
            // NOTE KI white causes least unexpectedly tinted results
            m_defaultMaterial = Material::createMaterial(BasicMaterial::white);
        }

        //{
        //    m_defaultMaterial.m_id = Material::DEFAULT_ID;
        //}

        m_forceDefaultMaterial = forceDefaultMaterial;

        loadData(meshSet);

        return !meshSet.isEmpty();
    }
}
