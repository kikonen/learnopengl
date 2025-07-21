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
    template <class T>
    class GLSyncQueue {
    public:
        GLSyncQueue(
            std::string_view name,
            size_t entryCount,
            size_t rangeCount,
            bool useMapped,
            bool useInvalidate,
            bool useFence,
            bool useFenceDebug);

        ~GLSyncQueue();

        size_t getEntryCount() const noexcept
        {
            return m_entryCount;
        }

        size_t getRangeCount() const noexcept
        {
            return m_rangeCount;
        }

        GLuint getFlags() const noexcept
        {
            return m_flags;
        }

        void setFlags(GLuint flags)
        {
            m_flags = flags;
        }

        GLuint getMapFlags() const noexcept
        {
            m_mapFlags;
        }

        void setMapFlags(GLuint flags)
        {
            m_mapFlags = flags;
        }

        // @param flags 0 for defaults
        // @param mapFlags 0 for defaults
        void prepare(
            int bindAlignment,
            bool debug);

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
        void set(int idx, const T& entry);

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
        // Process current, clear and set fence
        //
        void processCurrent(
            std::function<void(GLBufferRange&)> handle);

        //
        // Switch to next range
        // @return next buffer
        //
        GLBufferRange& next();

        // Bind whole buffer; offset of current buffer need to be used in rendering logic
        void bindSSBO(GLuint ssbo);

        void bindCurrentUBO(GLuint ubo, bool used, size_t count);
        void bindCurrentSSBO(GLuint ssbo, bool used, size_t count);

        void setFence();
        void waitFence();

    public:
        std::string m_name;
        GLBuffer m_buffer;

    private:
        const size_t m_entryCount;
        const size_t m_rangeCount;

        const bool m_useMapped;
        const bool m_useInvalidate;
        const bool m_useFence;
        const bool m_useFenceDebug;

        bool m_debug{ false };

        const size_t m_entrySize;

        GLuint m_flags;
        GLuint m_mapFlags;

        size_t m_bindAlignment = 0;
        size_t m_rangeLength = 0;
        size_t m_paddedRangeLength = 0;

        unsigned char* m_data{ nullptr };

        size_t m_current = 0;
        std::vector<GLBufferRange> m_ranges;
        std::vector<GLFence> m_fences;
    };
}
