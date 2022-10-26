#pragma once

#include "NodeController.h"

class VolumeController final : public NodeController
{
public:
    VolumeController();

    bool update(
        const RenderContext& ctx,
        Node& node,
        Node* parent) override;

    int getTarget();
    void setTarget(int targetID);

private:
    int m_targetID;
};
