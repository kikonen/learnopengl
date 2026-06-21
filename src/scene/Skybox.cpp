#include "Skybox.h"

#include "BrdfLutMaterial.h"
#include "SkyboxMaterial.h"

Skybox::Skybox()
{
}

Skybox::~Skybox() = default;

void Skybox::prepareRT(
    const PrepareContext& ctx)
{
    for (auto& material : m_materials) {
        if (material) {
            material->prepareRT(ctx);
        }
    }
}

void Skybox::bindTextures(kigl::GLState& state)
{
    for (auto& material : m_materials) {
        if (material) {
            material->bindTextures(state);
        }
    }
}


void Skybox::setMaterial(
    const util::Ref<SkyboxMaterial>& material,
    int index)
{
    m_materials[index] = material;
}
