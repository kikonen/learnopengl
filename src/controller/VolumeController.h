#pragma once

#include "NodeController.h"

class VolumeController final : public NodeController
{
public:
    VolumeController();

    virtual bool updateWT(
        const UpdateContext& ctx,
        Node& node) noexcept override;

    pool::NodeHandle getTargetId() const noexcept {
        return m_targetId;
    }

    void setTargetId(pool::NodeHandle targetId) noexcept {
        m_targetId = targetId;
    }

private:
    pool::NodeHandle m_targetId{};
};
