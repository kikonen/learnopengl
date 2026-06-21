#pragma once

#include <array>

#include "util/Ref.h"

struct PrepareContext;

class SkyboxMaterial;

namespace kigl
{
    class GLState;
}

class Skybox : public util::RefCountedSimple
{
public:
    Skybox();
    ~Skybox();

    void prepareRT(
        const PrepareContext& ctx);

    void bindTextures(kigl::GLState& state);

    const util::Ref<SkyboxMaterial>& getMaterial(int index) const noexcept
    {
        return m_materials[index];
    }

    void setMaterial(
        const util::Ref<SkyboxMaterial>& material,
        int index);

private:
    std::array<util::Ref<SkyboxMaterial>, 2> m_materials;
};
