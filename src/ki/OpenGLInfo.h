#pragma once

#include <string>
#include <fmt/format.h>

namespace ki {
    struct OpenGLInfo {
        std::string vendor;
        std::string renderer;
        std::string version;
        std::string glslVersion;

        int maxVertexUniformComponents = 0;
        int maxVertexAttributes = 0;

        // GL_MAX_COMPUTE_WORK_GROUP_COUNT
        int maxComputeWorkGroupCount[3] = { 0, 0, 0 };

        int preferredFormatRGBA8 = 0;
        int preferredFormatRGB8 = 0;

        std::string formatMaxComputeWorkGroupCount() const {
            return fmt::format("[{}, {}, {}]", maxComputeWorkGroupCount[0], maxComputeWorkGroupCount[1], maxComputeWorkGroupCount[2]);
        }
    };
}
