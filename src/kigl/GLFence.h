#pragma once

#include <string>

#include "kigl/kigl.h"

namespace kigl {
    struct GLFence {
        GLsync m_sync{ 0 };
        std::string m_name;

        GLFence(std::string_view name);

        bool isSet() const noexcept { return m_sync; }

        void setFence(bool debug);
        // @return true if fence is set now
        bool setFenceIfNotSet(bool debug);

        // https://www.cppstories.com/2015/01/persistent-mapped-buffers-in-opengl/
        void waitFence(bool debug);

        void waitFenceOnServer() const noexcept;
    };
}
