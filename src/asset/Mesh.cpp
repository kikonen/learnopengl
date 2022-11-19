#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(const std::string& name)
    : m_name(name)
{

}
Mesh::~Mesh()
{
    KI_INFO_SB("MESH: delete " + str());
}

const std::string Mesh::str() const
{
    return "<" + m_name + ">";
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
