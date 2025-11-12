#include "VaoRegistry.h"

#include <fmt/format.h>

#include "util/thread.h"
#include "util/Log.h"

#include "asset/Assets.h"

#include "kigl/GLState.h"

#include "mesh/vao/TexturedVAO.h"
#include "mesh/vao/SkinnedVAO.h"

#include "render/RenderContext.h"

namespace
{
    thread_local std::exception_ptr lastException = nullptr;

    static VaoRegistry* s_registry{ nullptr };
}

void VaoRegistry::init() noexcept
{
    assert(!s_registry);
    s_registry = new VaoRegistry();
}

void VaoRegistry::release() noexcept
{
    auto* s = s_registry;
    s_registry = nullptr;
    delete s;
}

VaoRegistry& VaoRegistry::get() noexcept
{
    assert(s_registry);
    return *s_registry;
}

VaoRegistry::VaoRegistry()
    : m_nullVao{ std::make_unique<kigl::GLVertexArray>() },
    m_texturedVao{ std::make_unique<mesh::TexturedVAO>("mesh_textured") },
    m_skinnedVao{ std::make_unique<mesh::SkinnedVAO>("mesh_skinned") },
    m_sharedPrimitiveVao{ std::make_unique<mesh::TexturedVAO>("shared_primitive") }
{
    m_dynamicPrimitiveVaos.push_back({ std::make_unique<mesh::TexturedVAO>("dynamic_primitive_1") });
    m_dynamicPrimitiveVaos.push_back({ std::make_unique<mesh::TexturedVAO>("dynamic_primitive_2") });
}

VaoRegistry::~VaoRegistry() {
}

void VaoRegistry::clear()
{
    ASSERT_RT();

    m_texturedVao->clear();
    m_skinnedVao->clear();
    m_sharedPrimitiveVao->clear();
    for (auto& vao : m_dynamicPrimitiveVaos) {
        vao->clear();
    }
}

void VaoRegistry::prepare()
{
    ASSERT_RT();

    // NOTE KI ensure id == 0 is not used for actual VAOs
    m_nullVao->create("NULL");
    assert(*m_nullVao < 255);

    m_texturedVao->prepare();
    m_skinnedVao->prepare();
    m_sharedPrimitiveVao->prepare();
    for (auto& vao : m_dynamicPrimitiveVaos) {
        vao->prepare();
    }

    assert(*m_texturedVao->getVAO() < 255);
    assert(*m_skinnedVao->getVAO() < 255);
    assert(*m_sharedPrimitiveVao->getVAO() < 255);
    assert(*m_dynamicPrimitiveVaos[0]->getVAO() < 255);
    assert(*m_dynamicPrimitiveVaos[1]->getVAO() < 255);
}

void VaoRegistry::updateRT(const UpdateContext& ctx)
{
    m_texturedVao->updateRT();
    m_skinnedVao->updateRT();
    m_sharedPrimitiveVao->updateRT();
    for (auto& vao : m_dynamicPrimitiveVaos) {
        vao->updateRT();
    }
}

void VaoRegistry::bindDefaultVao()
{
    kigl::GLState::get().bindVAO(m_texturedVao->getVaoId());
}
