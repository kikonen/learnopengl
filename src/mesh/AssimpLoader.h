#pragma once

#include <memory>
#include <atomic>

#include "asset/Material.h"

#include "mesh/ModelLoader.h"

struct aiScene;

namespace mesh {
    class AssimpLoader : public ModelLoader {
    public:
        AssimpLoader(
            std::shared_ptr<std::atomic<bool>> alive);

        ~AssimpLoader();

    protected:
        void loadData(
            ModelMesh& mesh);

    private:
        void processScene(
            ModelMesh& mesh,
            const aiScene* scene);
    };
}
