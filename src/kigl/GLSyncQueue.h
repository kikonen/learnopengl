#pragma once

#include <string>
#include <vector>
#include <functional>

#include <fmt/format.h>

#include "GLBuffer.h"
#include "GLBufferRange.h"
#include "GLFence.h"

namespace kigl {
    //
    // SyncQueue, which is split into multiple ranges, which can be fence synced
    //
    template <class T, bool mappedMode>
    class GLSyncQueue {
    public:
        GLSyncQueue(
            std::string_view name,
            size_t entryCount,
            size_t rangeCount,
            bool useFence,
            bool useSingleFence);

        ~GLSyncQueue();

        void prepare(int bindAlignment, bool debug);

        //
        // Send entry data, wait if needed for queue buffer
        //
        // @return true if current range is full (and ready to flush)
        //
        bool send(const T& entry);

        //
        // Set entry data, wait if needed for queue buffer
        //
        // NOTE KI with "set" it's upto caller to manage buffer full
        //
        void set(int idx, T& entry);

        inline GLBufferRange& current() {
            return m_ranges[m_current];
        }

        inline T& data() {
            auto& range = current();
            return (T&)m_data[range.m_baseOffset];
        }

        //
        // @return true if current range is full
        //
        inline bool full() const noexcept {
            return m_ranges[m_current].full();
        }

        inline bool empty() const noexcept {
            return m_ranges[m_current].empty();
        }

        inline void clear() {
            m_ranges[m_current].clear();
        }

        void flush();

        //
        // Process starting from oldest upto current
        //
        void processPending(
            std::function<void(GLBufferRange&)> handle,
            bool processCurrent,
            bool clear);

        //
        // Switch to next range
        // @return next buffer
        //
        GLBufferRange& next(bool fence);

        void bind(GLuint ubo, bool used, size_t count);
        void bindSSBO(GLuint ssbo, bool used, size_t count);

    private:
        void setFence(size_t index);
        void waitFence(size_t index);

    public:
        std::string m_name;
        GLBuffer m_buffer;

    private:
        const size_t m_entryCount;
        const size_t m_rangeCount;
        const bool m_useFence;
        const bool m_useSingleFence;
        bool m_debug{ false };

        const size_t m_entrySize;

        size_t m_bindAlignment = 0;
        size_t m_rangeLength = 0;
        size_t m_paddedRangeLength = 0;

        unsigned char* m_data{ nullptr };

        size_t m_current = 0;
        std::vector<GLBufferRange> m_ranges;
        std::vector<GLFence> m_fences;
    };
}
