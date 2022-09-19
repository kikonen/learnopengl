#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(const std::string& modelName)
    : modelName(modelName)
{

}
Mesh::~Mesh()
{
    KI_INFO_SB("MESH: delete");
}
