#pragma once

#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "Material.h"
#include "MaterialEntry.h"

class Mesh;

class MaterialVBO {
    friend class MeshType;

public:
    MaterialVBO() = default;
    virtual ~MaterialVBO() = default;

    void setMaterials(const std::vector<Material>& materials);
    const std::vector<Material>& getMaterials() const;

    virtual void prepareVAO(
        GLVertexArray& vao);

public:
    Material m_defaultMaterial;
    bool m_useDefaultMaterial = false;
    bool m_forceDefaultMaterial = false;

    GLBuffer* m_vbo{ nullptr };
    int m_offset = 0;

    std::vector<Material> m_materials;
    std::vector<MaterialEntry> m_entries;

private:
    bool m_prepared = false;

};
