#include "VaoRegistry.h"

#include <fmt/format.h>

#include "util/Log.h"

#include "asset/Assets.h"

#include "mesh/vao/TexturedVAO.h"
#include "mesh/vao/SkinnedVAO.h"

#include "render/RenderContext.h"

namespace {
    static VaoRegistry s_registry;
}

VaoRegistry& VaoRegistry::get() noexcept
{
    return s_registry;
}

VaoRegistry::VaoRegistry()
    : m_texturedVao{ std::make_unique<mesh::TexturedVAO>("mesh_textured") },
    m_skinnedVao{ std::make_unique<mesh::SkinnedVAO>("mesh_skinned") },
    m_primitiveVao{ std::make_unique<mesh::TexturedVAO>("mesh_primitive") }
{
}

VaoRegistry::~VaoRegistry() {
}

void VaoRegistry::prepare()
{
    m_texturedVao->prepare();
    m_skinnedVao->prepare();
    m_primitiveVao->prepare();
}

void VaoRegistry::updateRT(const UpdateContext& ctx)
{
    m_texturedVao->updateRT();
    m_skinnedVao->updateRT();
    m_primitiveVao->updateRT();
}
