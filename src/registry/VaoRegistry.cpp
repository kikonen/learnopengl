#include "VaoRegistry.h"

#include <fmt/format.h>

#include "util/Log.h"

#include "asset/Assets.h"

#include "mesh/vao/TexturedVAO.h"
#include "mesh/vao/SkinnedVAO.h"

#include "render/RenderContext.h"

namespace {
    static VaoRegistry g_registry;
}

VaoRegistry& VaoRegistry::get() noexcept
{
    return g_registry;
}

VaoRegistry::VaoRegistry()
    : m_nullVao{ std::make_unique<kigl::GLVertexArray>() },
    m_texturedVao{ std::make_unique<mesh::TexturedVAO>("mesh_textured") },
    m_skinnedVao{ std::make_unique<mesh::SkinnedVAO>("mesh_skinned") },
    m_sharedPrimitiveVao{ std::make_unique<mesh::TexturedVAO>("shared_primitive") },
    m_dynamicPrimitiveVao{ std::make_unique<mesh::TexturedVAO>("dynamic_primitive") }
{
}

VaoRegistry::~VaoRegistry() {
}

void VaoRegistry::prepare()
{
    // NOTE KI ensure id == 0 is not used for actual VAOs
    m_nullVao->create("NULL");

    m_texturedVao->prepare();
    m_skinnedVao->prepare();
    m_sharedPrimitiveVao->prepare();
    m_dynamicPrimitiveVao->prepare();
}

void VaoRegistry::updateRT(const UpdateContext& ctx)
{
    m_texturedVao->updateRT();
    m_skinnedVao->updateRT();
    m_sharedPrimitiveVao->updateRT();
    m_dynamicPrimitiveVao->updateRT();
}
