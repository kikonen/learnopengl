#include "Mesh.h"

#include "fmt/format.h"

namespace {
    int idBase = 0;

    std::mutex id_lock;

    int nextID()
    {
        std::lock_guard<std::mutex> lock(id_lock);
        return ++idBase;
    }
}

Mesh::Mesh()
    : m_objectID(nextID())
{
}

Mesh::~Mesh()
{
    KI_INFO_SB("MESH: delete " + str());
}

const std::string Mesh::str() const
{
    return fmt::format("<MESH: {}>", m_objectID);
}

void Mesh::prepareVolume() {
    const auto& aabb = calculateAABB();
    setAABB(aabb);

    setVolume(std::make_unique<Sphere>(
        (aabb.m_max + aabb.m_min) * 0.5f,
        // NOTE KI *radius* not diam needed
        glm::length(aabb.m_min - aabb.m_max) * 0.5f));
}

void Mesh::setVolume(std::unique_ptr<Volume> volume)
{
    m_volume = std::move(volume);
}

const Volume* Mesh::getVolume() const
{
    return m_volume.get();
}

void Mesh::setAABB(const AABB& aabb)
{
    m_aabb = aabb;
}

const AABB& Mesh::getAABB() const
{
    return m_aabb;
}
