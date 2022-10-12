#pragma once

#include "command/Command.h"

#include "model/Node.h"


class NodeCommand : public Command
{
public:
    NodeCommand(
        int objectID,
        float initialDelay,
        float finishTime);

    virtual bool isNode() override final { return true; };

    virtual void bind(const RenderContext& ctx, Node* node);

public:
    const int m_objectID;
    Node* m_node;
};
