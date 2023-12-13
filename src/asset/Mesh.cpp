#include "Mesh.h"

#include "fmt/format.h"

namespace {
    ki::mesh_id idBase = 0;

    std::mutex id_lock{};

    ki::mesh_id nextID()
    {
        std::lock_guard<std::mutex> lock(id_lock);
        return ++idBase;
    }
}

Mesh::Mesh()
    : m_id(nextID())
{
}

Mesh::~Mesh()
{
    KI_INFO(fmt::format("MESH: delete {}", str()));
}

const std::string Mesh::str() const noexcept
{
    return fmt::format("<MESH: id={}>", m_id);
}

void Mesh::prepareVolume() {
    setAABB(calculateAABB());
}

