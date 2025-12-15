#include "MeshSetLoader.h"

#include "util/util.h"

#include "mesh/MeshSet.h"


namespace mesh_set
{
    MeshSetLoader::MeshSetLoader(
        const std::shared_ptr<std::atomic_bool>& alive)
        : m_alive(alive)
    {
    }

    MeshSetLoader::~MeshSetLoader()
    {
    }

    bool MeshSetLoader::load(
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
