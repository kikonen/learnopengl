#pragma once

#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "Material.h"

class Mesh;
struct MaterialIndex;

class MaterialVBO {
    friend class MeshType;

public:
    MaterialVBO() = default;
    virtual ~MaterialVBO() = default;

    void setMaterials(const std::vector<Material>& materials);
    const std::vector<Material>& getMaterials() const;

    //virtual void prepareVAO(
    //    GLVertexArray& vao);

public:
    Material m_defaultMaterial;
    bool m_useDefaultMaterial = false;
    bool m_forceDefaultMaterial = false;

    bool m_singleMaterial = true;

    GLBuffer* m_buffer{ nullptr };
    int m_bufferIndex = 0;

    std::vector<Material> m_materials;
    std::vector<MaterialIndex> m_indeces;

private:
    bool m_prepared = false;

};
