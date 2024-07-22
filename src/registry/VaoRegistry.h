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

    mesh::TexturedVAO* getDebugVao()
    {
        return m_debugVao.get();
    }

private:
    std::unique_ptr<mesh::TexturedVAO> m_texturedVao;
    std::unique_ptr<mesh::SkinnedVAO> m_skinnedVao;

    std::unique_ptr<mesh::TexturedVAO> m_debugVao;
};
