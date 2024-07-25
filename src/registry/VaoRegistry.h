#pragma once

#include <vector>

#include "glm/glm.hpp"

#include "util/Util.h"
#include "kigl/GLVertexArray.h"

struct UpdateContext;

namespace mesh {
    class TexturedVAO;
    class SkinnedVAO;
}

class VaoRegistry {
public:
    static VaoRegistry& get() noexcept;

    VaoRegistry();
    VaoRegistry& operator=(const VaoRegistry&) = delete;

    ~VaoRegistry();

    void prepare();

    void updateRT(const UpdateContext& ctx);

    mesh::TexturedVAO* getTexturedVao()
    {
        return m_texturedVao.get();
    }

    mesh::SkinnedVAO* getSkinnedVao()
    {
        return m_skinnedVao.get();
    }

    // NOTE KI primitive VAO is "throwaway" render meshes
    // which are recalculated on every render cycle again
    // => mostly useful for debug thus
    mesh::TexturedVAO* getPrimitiveVao()
    {
        return m_primitiveVao.get();
    }

private:
    std::unique_ptr<mesh::TexturedVAO> m_texturedVao;
    std::unique_ptr<mesh::SkinnedVAO> m_skinnedVao;

    std::unique_ptr<mesh::TexturedVAO> m_primitiveVao;
};
