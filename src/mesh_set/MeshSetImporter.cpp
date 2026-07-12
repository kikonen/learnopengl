#include "MeshSetImporter.h"

#include "util/util.h"

#include "mesh/MeshSet.h"


namespace mesh_set
{
    MeshSetImporter::MeshSetImporter(
        const std::shared_ptr<std::atomic_bool>& alive)
        : m_alive(alive)
    {
    }

    MeshSetImporter::~MeshSetImporter()
    {
    }

    bool MeshSetImporter::load(
        mesh::MeshSet& meshSet,
        const util::Ref<Material>& defaultMaterial,
        bool forceDefaultMaterial)
    {
        if (defaultMaterial) {
            // NOTE KI keep shared reference
            m_defaultMaterial = defaultMaterial;
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

        return !meshSet.empty();
    }
}
