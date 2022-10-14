#pragma once

#include "command/Command.h"

#include "model/Node.h"


class NodeCommand : public Command
{
public:
    NodeCommand(
        int afterCommandId,
        int objectID,
        float initialDelay,
        float finishTime,
        bool relative);

    virtual bool isNode() override final { return true; };

    virtual void bind(const RenderContext& ctx, Node* node);

public:
    const int m_objectID;
    const bool m_relative;

    Node* m_node;
};
