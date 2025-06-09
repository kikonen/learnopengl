#include "SelectionRegistry.h"

#include <algorithm>

#include <fmt/format.h>

#include "util/debug.h"
#include "util/thread.h"
#include "util/log.h"
#include "ki/limits.h"
#include "kigl/kigl.h"

#include "asset/Assets.h"

#include "model/Node.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "script/CommandEngine.h"
#include "script/command/RotateNode.h"

#include "render/DebugContext.h"

#include "material/Material.h"

#include "registry/Registry.h"


namespace
{
    thread_local std::exception_ptr lastException = nullptr;

    static SelectionRegistry* s_registry{ nullptr };
}

void SelectionRegistry::init() noexcept
{
    assert(!s_registry);
    s_registry = new SelectionRegistry();
}

void SelectionRegistry::release() noexcept
{
    auto* s = s_registry;
    s_registry = nullptr;
    delete s;
}

SelectionRegistry& SelectionRegistry::get() noexcept
{
    assert(s_registry);
    return *s_registry;
}

SelectionRegistry::SelectionRegistry() = default;
SelectionRegistry::~SelectionRegistry() = default;

void SelectionRegistry::clear()
{
    m_selected.clear();
    m_tagged.clear();
}

void SelectionRegistry::prepare(Registry* registry)
{
    const auto& assets = Assets::get();

    m_registry = registry;

    {
        m_selectionMaterial = std::make_unique<Material>();
        *m_selectionMaterial = Material::createMaterial(BasicMaterial::selection);
        m_selectionMaterial->registerMaterial();
        m_selectionMaterialIndex = m_selectionMaterial->m_registeredIndex;
    }

    {
        m_tagMaterial = std::make_unique<Material>();
        *m_tagMaterial = Material::createMaterial(BasicMaterial::highlight);
        m_tagMaterial->registerMaterial();
        m_tagMaterialIndex = m_tagMaterial->m_registeredIndex;
    }

    attachListeners();
}

void SelectionRegistry::attachListeners()
{
    auto* dispatcherView = m_registry->m_dispatcherView;

    dispatcherView->addListener(
        event::Type::node_select,
        [this](const event::Event& e) {
            const auto& data = e.body.select;
            if (auto handle = pool::NodeHandle::toHandle(data.target)) {
                if (data.select) {
                    selectNode(handle, data.append);
                }
                else {
                    deselectNode(handle);
                }
            }
        });
}

void SelectionRegistry::selectNode(pool::NodeHandle handle, bool append)
{
    const auto& dbg = render::DebugContext::get();

    if (!append) {
        m_selected.clear();
    }

    if (!handle) return;
    if (isSelected(handle)) return;

    m_selected.push_back(handle);

    KI_INFO(fmt::format("selected: {}", handle.str()));

    // NOTE KI fancy test/debug
    if (dbg.m_selectionAxis != glm::vec3{ 0.f }) {
        auto& commandEngine = script::CommandEngine::get();
        commandEngine.addCommand(
            0,
            script::RotateNode{
                handle,
                5,
                true,
                dbg.m_selectionAxis,
                360.f
            });
    }
}

void SelectionRegistry::deselectNode(pool::NodeHandle handle)
{
    if (!handle) return;
    auto it = std::remove(m_selected.begin(), m_selected.end(), handle);
    m_selected.erase(it, m_selected.end());
}

void SelectionRegistry::clearSelection()
{
    m_selected.clear();
}

void SelectionRegistry::tagNode(pool::NodeHandle handle, uint8_t tag)
{
    if (!handle) return;
    m_tagged.push_back(handle);
}

void SelectionRegistry::untagNode(pool::NodeHandle handle, uint8_t tag)
{
    if (!handle) return;
    const auto& it = std::remove(m_tagged.begin(), m_tagged.end(), handle);
    m_tagged.erase(it, m_tagged.end());
}

void SelectionRegistry::clearTagged()
{
    m_tagged.clear();
}

const Material& SelectionRegistry::getSelectionMaterial() const noexcept
{
    return *m_selectionMaterial;
}

void SelectionRegistry::setSelectionMaterial(const Material& material)
{
    *m_selectionMaterial = material;
}

int SelectionRegistry::getHighlightIndex(pool::NodeHandle handle) const noexcept
{
    if (isSelected(handle)) return m_selectionMaterialIndex;
    if (isTagged(handle)) return m_tagMaterialIndex;
    return 0;
}
