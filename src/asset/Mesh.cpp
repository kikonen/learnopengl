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
