#include "SpriteMesh.h"

#include <fmt/format.h>

#include "ki/GL.h"

#include "asset/Sphere.h"
#include "asset/SpriteMaterialInit.h"
#include "asset/SpriteVAO.h"

#include "registry/MeshType.h"


namespace {

    const AABB SPRITE_AABB = { glm::vec3{ -1.f, -1.f, 0.f }, glm::vec3{ 1.f, 1.f, 0.f }, true };

    SpriteVAO spriteVAO;
}


SpriteMesh::SpriteMesh()
    : Mesh()
{
}

SpriteMesh::~SpriteMesh()
{
}

const std::string SpriteMesh::str() const noexcept
{
    return fmt::format("<SPRITE: id={}>", m_objectID);
}

const AABB SpriteMesh::calculateAABB() const
{
    return SPRITE_AABB;
}

const std::vector<Material>& SpriteMesh::getMaterials() const
{
    return m_material;
}

GLVertexArray* SpriteMesh::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return m_vao;
    m_prepared = true;

    m_vao = spriteVAO.prepare();
    return m_vao;
}

void SpriteMesh::prepareMaterials(
    MaterialVBO& materialVBO)
{
    SpriteMaterialInit init;
    init.prepare(*this, materialVBO);
}

void SpriteMesh::prepareDrawOptions(
    backend::DrawOptions& drawOptions)
{
    drawOptions.type = backend::DrawOptions::Type::arrays;
    drawOptions.mode = GL_POINTS;
    drawOptions.indexFirst = 0;
    drawOptions.indexCount = 1;
}
