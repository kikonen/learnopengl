#pragma once

#include <string>

#include "asset/AABB.h"

#include "NodeGenerator.h"

#include "backend/DrawOptions.h"

#include "mesh/ModelVBO.h"
#include "mesh/ModelVAO.h"

#include "text/size.h"

namespace text {
    class TextDraw;
}

class TextGenerator final : public NodeGenerator {
public:
    TextGenerator();

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
        SnapshotRegistry& snapshotRegistry,
        EntityRegistry& entityRegistry,
        Node& container) override;

    virtual void updateVAO(
        const RenderContext& ctx,
        const Node& container);

    virtual void bindBatch(
        const RenderContext& ctx,
        mesh::MeshType* type,
        Node& container,
        render::Batch& batch) override;

    virtual const kigl::GLVertexArray* getVAO(
        const Node& container) const noexcept override;

    text::font_id getFontId() const noexcept { return m_fontId; }

    void setFontId(text::font_id fontId) noexcept {
        m_fontId = fontId;
        m_dirty = true;
    }

    std::string getText() const noexcept { return m_text; }

    void setText(std::string_view text) {
        m_text = text;
        m_dirty = true;
    }

    void clear();

private:
    bool m_dirty{ true };

    AABB m_aabb;

    mesh::ModelVAO m_vao{ "text" };
    mesh::ModelVBO m_vbo;

    std::unique_ptr<text::TextDraw> m_draw;

    text::font_id m_fontId{ 0 };
    std::string m_text;
};
