#pragma once

#include <string>

#include "kigl/kigl.h"

namespace kigl {
    struct GLFence {
        GLsync m_sync{ 0 };
        std::string m_name;

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

        void swap(GLFence& o) noexcept;

        void discard();

        bool isSet() const noexcept { return m_sync; }

        void setFence(bool debug);
        // @return true if fence is set now
        bool setFenceIfNotSet(bool debug);

        // https://www.cppstories.com/2015/01/persistent-mapped-buffers-in-opengl/
        void waitFence(bool debug);

        void waitFenceOnServer() const noexcept;
    };
}
