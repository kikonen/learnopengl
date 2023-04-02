#include "Mesh.h"

#include "fmt/format.h"

namespace {
    int idBase = 0;

    std::mutex id_lock{};

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
    KI_INFO(fmt::format("MESH: delete {}", str()));
}

const std::string Mesh::str() const noexcept
{
    return fmt::format("<MESH: id={}>", m_objectID);
}

void Mesh::prepareVolume() {
    setAABB(calculateAABB());
}

