#pragma once

#include <string>
#include <string_view>

#include "NodeController.h"

// Drives a text node's TextGenerator with the scene World's formatted clock
// ("YYYY-MM-DD HH:MM"). Runs on WT; TextGenerator::setText is mutex-guarded, and
// the text is only pushed when it actually changes (minute resolution).
class ClockController final : public NodeController
{
public:
    // fmtSpec: strftime-style chrono spec (e.g. "%H:%M"); empty -> default
    ClockController(std::string_view fmtSpec);

    virtual bool updateWT(
        const UpdateContext& ctx,
        model::Node& node) noexcept override;

private:
    const std::string m_format;
    std::string m_last;
};
