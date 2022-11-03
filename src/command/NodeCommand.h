#pragma once

#include "command/Command.h"

#include "model/Node.h"


class NodeCommand : public Command
{
public:
    NodeCommand(
        int afterCommandId,
        int objectID,
        float finishTime,
        bool relative) noexcept;

    virtual bool isNode() noexcept override final { return true; };

    virtual void bind(const RenderContext& ctx, Node* node) noexcept;

public:
    const int m_objectID;
    const bool m_relative;

    Node* m_node;
};
