#include "ModelLoader.h"

#include "util/Util.h"

#include "asset/Assets.h"

#include "mesh/MeshType.h"
#include "mesh/ModelMesh.h"


namespace mesh {
    ModelLoader::ModelLoader(
        std::shared_ptr<std::atomic<bool>> alive)
        : m_alive(alive)
    {
    }

    ModelLoader::~ModelLoader()
    {
    }

    ModelMesh* ModelLoader::load(
        ModelMesh& mesh,
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

        m_forceDefaultMaterial = forceDefaultMaterial;

        if (!mesh.m_loaded) {
            loadData(mesh);
            mesh.m_loaded = true;
            mesh.m_valid = !mesh.m_indeces.empty();
        }

        return mesh.m_valid ? &mesh : nullptr;
    }
}
