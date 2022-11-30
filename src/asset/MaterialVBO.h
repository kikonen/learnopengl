#pragma once

#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "Material.h"

struct Vertex;
class ModelMesh;

class MaterialVBO {
    friend class MeshType;

public:
    void setMaterials(const std::vector<Material>& materials);
    const std::vector<Material>& getMaterials() const;

    void create();

    void prepare(
        ModelMesh& mesh);

    void prepareVAO(
        GLVertexArray& vao);

private:
    void prepareVBO(
        std::vector<Vertex>& vertices);

public:
    Material m_defaultMaterial;
    bool m_useDefaultMaterial = false;
    bool m_forceDefaultMaterial = false;


    GLBuffer m_vbo;

private:
    bool m_prepared = false;

    std::vector<Material> m_materials;
};
