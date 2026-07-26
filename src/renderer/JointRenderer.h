#pragma once

#include <memory>

#include "util/Ref.h"

#include "MeshRenderer.h"

namespace mesh {
    class Mesh;
}

class JointRenderer : public MeshRenderer
{
public:
    JointRenderer();
    ~JointRenderer();

    virtual void prepareRT(const PrepareContext& ctx) override;

protected:
    void updateImpl(
        const UpdateContext& ctx) override;

private:
    util::Ref<mesh::Mesh> m_mesh;
    ki::material_index m_materialIndex{ 0 };
};
