#pragma once

#include <string>
#include <vector>

#include "material/Material.h"
#include "asset/AABB.h"

#include "NodeGenerator.h"

#include "backend/DrawOptions.h"

#include "text/size.h"
#include "text/Align.h"

namespace mesh {
    class TextMesh;
    struct LodMesh;
}

namespace text {
    class TextDraw;
}

class TextGenerator final : public NodeGenerator {
public:
    TextGenerator();

    ~TextGenerator();

    virtual void prepareWT(
        const PrepareContext& ctx,
        model::Node& container) override;

    virtual void prepareRT(
        const PrepareContext& ctx,
        model::Node& container) override;

    virtual void updateWT(
        const UpdateContext& ctx,
        const model::Node& container) override;

    virtual void updateVAO(
        const RenderContext& ctx,
        const model::Node& container);

    virtual void bindBatch(
        const RenderContext& ctx,
        const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
        const std::function<void(ki::program_id)>& programPrepare,
        uint8_t kindBits,
        render::Batch& batch,
        const model::Node& container,
        const Snapshot& snapshot) override;

    virtual const std::vector<mesh::LodMesh>* getLodMeshes(const model::Node& container) const
    {
        return &m_lodMeshes;
    }

    text::font_id getFontId() const noexcept { return m_fontId; }

    void setFontId(text::font_id fontId) noexcept {
        if (m_fontId != fontId) {
            m_fontId = fontId;
            m_dirty = true;
        }
    }

    std::string getText() const noexcept { return m_text; }

    void setText(std::string_view text) {
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

    GLuint64 getAtlasTextureHandle() const noexcept;

    void clear();

public:
    Material m_material;

private:
    bool m_dirty{ true };

    AABB m_aabb;

    std::shared_ptr<mesh::TextMesh> m_mesh;
    std::vector<mesh::LodMesh> m_lodMeshes;
    std::unique_ptr<text::TextDraw> m_draw;

    text::font_id m_fontId{ 0 };

    glm::vec2 m_pivot{ 0.f };
    text::Align m_alignHorizontal{ text::Align::none };
    text::Align m_alignVertical{ text::Align::none };

    std::string m_text;
};
