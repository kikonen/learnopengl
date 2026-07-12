#pragma once

#include <string>
#include <vector>
#include <mutex>

#include "util/Ref.h"

#include "asset/AABB.h"

#include "material/Material.h"

#include "NodeGenerator.h"

#include "backend/DrawOptions.h"

#include "text/size.h"
#include "text/Align.h"


namespace mesh
{
    class TextMesh;
    struct LodMesh;
}

namespace render
{
    struct DrawableInfo;
}

namespace text
{
    class TextDraw;
}

class TextGenerator final : public NodeGenerator
{
public:
    TextGenerator();

    ~TextGenerator();

    void prepareWT(
        const PrepareContext& ctx,
        model::Node& container) override;

    void prepareRT(
        const PrepareContext& ctx,
        model::Node& container,
        const model::Snapshot& snapshot) override;

    void updateRT(
        const UpdateContext& ctx,
        const model::Node& container) override;

    void updateMaterial(const model::Node& container);

    void registerDrawables(
        render::InstanceRegistry& instanceRegistry,
        const model::Node& container,
        const model::Snapshot& snapshot) override;

    void updateDrawables(
        render::InstanceRegistry& instanceRegistry,
        const model::Node& container,
        const model::Snapshot& snapshot) override;

    text::font_id getFontId() const noexcept { return m_fontId; }

    void setFontId(text::font_id fontId) noexcept {
        if (m_fontId != fontId) {
            m_fontId = fontId;
            m_dirty = true;
        }
    }

    std::string getText() const noexcept {
        std::lock_guard lock{ m_lock };
        return m_text;
    }

    void setText(std::string_view text) {
        std::lock_guard lock{ m_lock };
        if (m_text != text) {
            m_text = text;
            m_dirty = true;
        }
    }

    void setPivot(const glm::vec2& pivot) {
        if (m_pivot != pivot) {
            m_pivot = pivot;
            m_dirty = true;
        }
    }

    void setAlignHorizontal(text::Align align) {
        if (m_alignHorizontal != align) {
            m_alignHorizontal = align;
            m_dirty = true;
        }
    }

    void setAlignVertical(text::Align align) {
        if (m_alignVertical != align) {
            m_alignVertical = align;
            m_dirty = true;
        }
    }

    uint32_t getMaxSize() const noexcept {
        return m_maxSize;
    }

    void setMaxSize(uint32_t maxSize) noexcept {
        m_maxSize = maxSize;
    }

    GLuint64 getAtlasTextureHandle() const noexcept;

    void clear();

private:
    bool m_dirty{ true };
    mutable std::mutex m_lock{};

    std::unique_ptr<text::TextDraw> m_draw;

    util::Ref<mesh::LodMesh> m_lodMesh;
    util::Ref<mesh::TextMesh> m_mesh;

    text::font_id m_fontId{ 0 };

    glm::vec2 m_pivot{ 0.f };
    text::Align m_alignHorizontal{ text::Align::none };
    text::Align m_alignVertical{ text::Align::none };

    std::string m_text;

    uint32_t m_maxSize;

    bool m_fontRegistered{ false };
};
