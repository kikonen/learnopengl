#include "kigl/OpenGLInfo.h"

#include <regex>
#include <algorithm>

#include <fmt/format.h>

#include "asset/Assets.h"

#include "util/Log.h"
#include "util/Util.h"

#include "kigl/kigl.h"

namespace kigl
{
    bool OpenGLInfo::isNvidia() const
    {
        const auto vendor = util::toLower(m_vendor);
        return std::regex_match(vendor, std::regex(".*nvidia.*"));
    }

    bool OpenGLInfo::isIntel() const
    {
        const auto vendor = util::toLower(m_vendor);
        return std::regex_match(vendor, std::regex(".*intel.*"));
    }

    void OpenGLInfo::dumpInfo() const
    {
        const auto& assets = Assets::get();

        // NOTE KI https://www.khronos.org/opengl/wiki/Common_Mistakes
        // - preferredFormat is performnce topic
        KI_INFO_OUT(fmt::format(
            R"(
ENGINE::RUN
-------------
vendor:   {}
renderer: {}
version:  {}
glsl:     {}
-------------
GL_MAX_VERTEX_UNIFORM_COMPONENTS:  {}
GL_MAX_VERTEX_ATTRIBS:             {}
GL_MAX_COMPUTE_WORK_GROUP_COUNT:   {}
GL_PREFERRED_TEXTURE_FORMAT_RGBA8: 0x{:x}
GL_PREFERRED_TEXTURE_FORMAT_RGB8:  0x{:x}
)",
m_vendor,
m_renderer,
m_version,
m_glslVersion,
m_maxVertexUniformComponents,
m_maxVertexAttributes,
formatMaxComputeWorkGroupCount(),
m_preferredFormatRGBA8,
m_preferredFormatRGB8));

        KI_INFO("[EXTENSIONS]");
        const auto& extensions = kigl::GL::getExtensions();
        for (const auto& ext : extensions) {
            KI_INFO(ext);
        }

        {
            //// Check ASTC support
            //KI_INFO("[ASTC_SUPPORT]");
            //const char* ext = (const char*)glGetString(GL_EXTENSIONS);
            //KI_INFO_OUT(fmt::format(
            //    "ASTC LDR: {}",
            //    strstr(ext, "GL_KHR_texture_compression_astc_ldr") != nullptr));

            //KI_INFO_OUT(fmt::format(
            //    "ASTC HDR: {}",
            //    strstr(ext, "GL_KHR_texture_compression_astc_hdr") != nullptr));

            // Dump all compressed formats
            KI_INFO("[COMPRESSED_FORMATS]");
            GLint count;
            glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &count);
            std::vector<GLint> formats(count);
            glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, formats.data());
            for (int i = 0; i < count; i++) {
                KI_INFO_OUT(fmt::format("0x{:04X}", formats[i]));
            }
        }
    }
}
