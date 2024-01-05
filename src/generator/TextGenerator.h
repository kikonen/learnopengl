#pragma once

#include "NodeGenerator.h"

class TextGenerator final : public NodeGenerator {
public:
    TextGenerator();

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& container) override;

    virtual void update(
        const UpdateContext& ctx,
        Node& container) override;

    virtual void updateVAO(
        const RenderContext& ctx,
        const Node& container);

    virtual const kigl::GLVertexArray* getVAO(
        const Node& container) const noexcept override;

    virtual const backend::DrawOptions& getDrawOptions(
        const Node& container) const noexcept override;
};
