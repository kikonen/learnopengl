#pragma once

#include <string>
#include <memory>
#include <atomic>

#include "glm/glm.hpp"

#include "asset/Material.h"

namespace mesh {
    class ModelMesh;

    class ModelLoader
    {
    public:
        ModelLoader(
            std::shared_ptr<std::atomic<bool>> alive);

        virtual ~ModelLoader();

        // @return pointer to mesh if load was success
        ModelMesh* load(
            ModelMesh& mesh,
            Material* defaultMaterial,
            bool forceDefaultMaterial);

    protected:
        virtual void loadData(
            ModelMesh& mesh) = 0;

    protected:
        Material m_defaultMaterial;
        bool m_forceDefaultMaterial = false;

        std::shared_ptr<std::atomic<bool>> m_alive;
    };
}
