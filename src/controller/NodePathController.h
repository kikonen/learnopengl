#pragma once

#include "NodeController.h"

class NodePathController final : public NodeController
{
public:
    NodePathController(
        const glm::vec3& center,
        int pathMode);

    bool update(
        const RenderContext& ctx,
        Node& node,
        Node* parent) override;

private:
    const glm::vec3 center;
    const int pathMode;

    bool m_translate;
    bool m_rotate;
    bool m_scale;
};

