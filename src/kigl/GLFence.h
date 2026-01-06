#pragma once

#include <string>

#include "kigl/kigl.h"

namespace kigl {
    struct GLFence {
        GLsync m_sync{ 0 };
        std::string m_name;

        GLFence() {}
        GLFence(std::string_view name);

        GLFence(GLFence& o) = delete;
        GLFence(const GLFence& o) = delete;
        GLFence(GLFence&& o) noexcept
        {
            swap(o);
        }

        ~GLFence();

        GLFence& operator=(GLFence& o) = delete;
        GLFence& operator=(GLFence&& o) noexcept
        {
            GLFence tmp(std::move(o));
            swap(tmp);
            return *this;
        }

        bool opreator() const noexcept { return m_sync; }

        void swap(GLFence& o) noexcept;

        void release();

        bool isSet() const noexcept { return m_sync; }

        void setFence();
        void setFence(bool debug) { setFence(); }

        // @return true if fence is set now
        bool setFenceIfNotSet();
        bool setFenceIfNotSet(bool debug) { return setFenceIfNotSet(); }

        // https://www.cppstories.com/2015/01/persistent-mapped-buffers-in-opengl/
        void waitFence();
        void waitFence(bool) { waitFence(); }

        void waitFenceOnServer() const noexcept;
    };
}
