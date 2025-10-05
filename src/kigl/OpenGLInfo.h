#pragma once

#include <string>
#include <fmt/format.h>

namespace kigl {
    struct OpenGLInfo {
        std::string m_vendor;
        std::string m_renderer;
        std::string m_version;
        std::string m_glslVersion;

        int m_maxVertexUniformComponents = 0;
        int m_maxVertexAttributes = 0;

        // GL_MAX_COMPUTE_WORK_GROUP_COUNT
        int m_maxComputeWorkGroupCount[3] = { 0, 0, 0 };

        int m_maxTextureUnits = 0;

        int m_preferredFormatRGBA8 = 0;
        int m_preferredFormatRGB8 = 0;

        std::string formatMaxComputeWorkGroupCount() const {
            return fmt::format(
                "[{}, {}, {}]",
                m_maxComputeWorkGroupCount[0],
                m_maxComputeWorkGroupCount[1],
                m_maxComputeWorkGroupCount[2]);
        }

        bool isNvidia() const;
        bool isIntel() const;

        void dumpInfo() const;
    };
}
