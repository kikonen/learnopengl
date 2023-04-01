#pragma once

#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "Material.h"


class Mesh;

class MaterialVBO {
    friend class MeshType;

public:
    MaterialVBO() = default;
    virtual ~MaterialVBO() = default;

    void setMaterials(const std::vector<Material>& materials);
    const std::vector<Material>& getMaterials() const noexcept;

    const Material& getFirst() const noexcept;

    bool isSingle() const noexcept { return m_indeces.empty(); }

public:
    Material m_defaultMaterial;
    bool m_useDefaultMaterial = false;
    bool m_forceDefaultMaterial = false;

    //GLBuffer* m_buffer{ nullptr };
    int m_bufferIndex = 0;

    std::vector<Material> m_materials;
    std::vector<GLuint> m_indeces;

private:
    bool m_prepared = false;

};
