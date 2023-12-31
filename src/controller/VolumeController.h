#pragma once

#include "NodeController.h"

class VolumeController final : public NodeController
{
public:
    VolumeController();

    virtual bool updateWT(
        const UpdateContext& ctx,
        Node& node) noexcept override;

    int getTarget();
    void setTarget(int targetID);

private:
    int m_targetID{ 0 };
};
