#pragma once

#include <vector>

#include "glm/glm.hpp"

#include "util/util.h"
#include "kigl/GLVertexArray.h"

#include "mesh/vao/TexturedVAO.h"

struct UpdateContext;

namespace mesh {
    class TexturedVAO;
    class SkinnedVAO;
}

class VaoRegistry {
public:
    static void init() noexcept;
    static void release() noexcept;
    static VaoRegistry& get() noexcept;

    VaoRegistry();
    VaoRegistry& operator=(const VaoRegistry&) = delete;

    ~VaoRegistry();

    void clear();
    void prepare();

    void updateRT(const UpdateContext& ctx);

    void bindDefaultVao();

    mesh::TexturedVAO* getTexturedVao()
    {
        return m_texturedVao.get();
    }

    mesh::SkinnedVAO* getSkinnedVao()
    {
        return m_skinnedVao.get();
    }

    // Primitives which can be shared acros several incocations
    // semi-permanent, but VAP can be flushed sometimes
    mesh::TexturedVAO* getSharedPrimitiveVao()
    {
        return m_sharedPrimitiveVao.get();
    }

    // NOTE KI primitive VAO is "throwaway" render meshes
    // which are recalculated on every render cycle again
    // => mostly useful for debug thus
    mesh::TexturedVAO* getDynamicPrimitiveVao(int index)
    {
        return m_dynamicPrimitiveVaos[index].get();
    }

private:
    std::unique_ptr<kigl::GLVertexArray> m_nullVao;

    std::unique_ptr<mesh::TexturedVAO> m_texturedVao;
    std::unique_ptr<mesh::SkinnedVAO> m_skinnedVao;

    std::unique_ptr<mesh::TexturedVAO> m_sharedPrimitiveVao;
    std::vector<std::unique_ptr<mesh::TexturedVAO>> m_dynamicPrimitiveVaos;
};
