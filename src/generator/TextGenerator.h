#pragma once

#include <string>

#include "asset/Material.h"
#include "asset/AABB.h"

#include "NodeGenerator.h"

#include "backend/DrawOptions.h"

#include "text/size.h"

namespace mesh {
    class TextMesh;
}

namespace text {
    class TextDraw;
}

class TextGenerator final : public NodeGenerator {
public:
    TextGenerator();

    ~TextGenerator();

    virtual void prepare(
        const PrepareContext& ctx,
        Node& container) override;

    virtual void prepareRT(
        const PrepareContext& ctx,
        Node& container) override;

    virtual void updateWT(
        const UpdateContext& ctx,
        Node& container) override;

    virtual void updateEntity(
        NodeSnapshotRegistry& snapshotRegistry,
        EntityRegistry& entityRegistry,
        Node& container) override;

    virtual void updateVAO(
        const RenderContext& ctx,
        const Node& container);

    virtual void bindBatch(
        const RenderContext& ctx,
        mesh::MeshType* type,
        const std::function<Program* (const mesh::LodMesh&)>& programSelector,
        uint8_t kindBits,
        render::Batch& batch,
        Node& container) override;

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

    GLuint64 getAtlasTextureHandle() const noexcept;

    void clear();

public:
    Material m_material;

private:
    bool m_dirty{ true };

    AABB m_aabb;

    std::unique_ptr<text::TextDraw> m_draw;

    text::font_id m_fontId{ 0 };
    std::string m_text;
};
