#pragma once

#include "NodeController.h"

class VolumeController final : public NodeController
{
public:
    VolumeController();

    virtual bool updateWT(
        const UpdateContext& ctx,
        Node& node) noexcept override;

    ki::node_id getTargetId();
    void setTargetId(int targetId);

private:
    ki::node_id m_targetId{ 0 };
};
