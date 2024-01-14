#pragma once

#include <string>

#include "NodeGenerator.h"

#include "backend/DrawOptions.h"

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

    virtual void update(
        const UpdateContext& ctx,
        Node& container) override;

    virtual void updateVAO(
        const RenderContext& ctx,
        const Node& container);

    virtual void bindBatch(
        const RenderContext& ctx,
        Node& container,
        render::Batch& batch) override;

    virtual const kigl::GLVertexArray* getVAO(
        const Node& container) const noexcept override;

    virtual const backend::DrawOptions& getDrawOptions(
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

private:
    bool m_dirty{ true };

    backend::DrawOptions m_drawOptions;
    std::unique_ptr<text::TextDraw> m_draw;

    text::font_id m_fontId{ 0 };
    std::string m_text;
};
