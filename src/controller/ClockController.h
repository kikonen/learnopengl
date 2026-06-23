#pragma once

#include <string>

#include "NodeController.h"

// Drives a text node's TextGenerator with the scene World's formatted clock
// ("YYYY-MM-DD HH:MM"). Runs on WT; TextGenerator::setText is mutex-guarded, and
// the text is only pushed when it actually changes (minute resolution).
class ClockController final : public NodeController
{
public:
    ClockController();

    virtual bool updateWT(
        const UpdateContext& ctx,
        model::Node& node) noexcept override;

private:
    std::string m_last;
};
