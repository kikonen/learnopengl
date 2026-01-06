#pragma once

#include <fmt/format.h>

#include "kigl/kigl.h"

namespace kigl {
    struct GLBuffer {
        GLBuffer(std::string_view name)
            : m_name{ name }
        {
        }

        GLBuffer(GLBuffer& o) = delete;
        GLBuffer(const GLBuffer& o) = delete;
        GLBuffer(GLBuffer&& o) noexcept
        {
            swap(o);
        }

        ~GLBuffer() {
            discard();
        }

        GLBuffer& operator=(GLBuffer& o) = delete;
        GLBuffer& operator=(GLBuffer&& o) noexcept
        {
            GLBuffer tmp(std::move(o));
            swap(tmp);
            return *this;
        }

        operator int() const { return m_id; }
        GLuint id() const { return m_id; }

        void swap(GLBuffer& o) noexcept;

        void discard() {
            if (m_mapped) {
                glUnmapNamedBuffer(m_id);
            }
            if (m_created) {
                glDeleteBuffers(1, &m_id);
            }
            m_mapped = false;
            m_created = false;
            m_id = 0;
        }

        size_t size() const noexcept
        {
            return m_size;
        }

        bool isCreated() const noexcept
        {
            return m_created;
        }

        bool isMapped() const noexcept
        {
            return m_mapped;
        }

        void create() {
            if (m_created) return;

            glCreateBuffers(1, &m_id);
            m_created = true;

            kigl::setLabel(GL_BUFFER, m_id, m_name);
            KI_DEBUG(fmt::format("BUFFER: create - name={}, id={}", m_name, m_id));
        }

        void createEmpty(size_t size, GLuint flags) {
            create();
            initEmpty(size, flags);
        }

        // NOTE KI discards current buffer and re-creates it
        // => *MUST* rebind all relevant bindings
        //
        // @return true if recreate was done
        //
        bool resizeBuffer(
            size_t size,
            bool keepData)
        {
            KI_DEBUG(fmt::format(
                "BUFFER: resize - name={}, id={}, oldSize={}, newSize={}",
                m_name, m_id,
                m_size, size));
            if (size == m_size) return false;
            unmap();

            bool wasCreated = m_created;
            GLuint oldId = m_id;
            size_t oldSize = m_size;
            m_created = false;

            createEmpty(size, m_flags);

            if (wasCreated) {
                if (keepData && m_usedSize > 0) {
                    glCopyNamedBufferSubData(
                        oldId,
                        m_id,
                        0,
                        0,
                        std::min(m_usedSize, size));
                }

                glDeleteBuffers(1, &oldId);
            }

            return true;
        }

        // For mapped buffer
        // flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        void initEmpty(size_t size, GLuint flags) {
            m_size = size;
            m_flags = flags;

            KI_DEBUG(fmt::format(
                "BUFFER: initEmpty - name={}, id={}, size={}, flags={}",
                m_name, m_id, m_size, flags));

            glNamedBufferStorage(m_id, size, nullptr, flags);
        }

        void init(size_t size, const void* data, GLuint flags) {
            m_size = size;

            KI_DEBUG(fmt::format(
                "BUFFER: init - name={}, id={}, size={}, flags={}",
                m_name, m_id, m_size, flags));

            glNamedBufferStorage(m_id, size, data, flags);
        }

        void markUsed(size_t usedSize)
        {
            assert(usedSize <= m_size);
            m_usedSize = std::min(usedSize, m_size);
        }


        void clear() {
            glClearNamedBufferSubData(
                m_id,
                GL_R32UI,
                0,
                m_size,
                GL_RED_INTEGER,
                GL_UNSIGNED_INT,
                nullptr);
        }

        void update(size_t offset, size_t length, const void* data) {
            if (length <= 0) return;
            glNamedBufferSubData(m_id, offset, length, data);
        }

        void get(void* data) {
            glGetNamedBufferSubData(m_id, 0, m_size, data);
        }

        void getRange(size_t offset, size_t length, void* data) {
            glGetNamedBufferSubData(m_id, offset, length, data);
        }

        void bindUBO(GLuint ubo) {
            bindUBORange(ubo, 0, m_size);
        }

        void bindUBORange(GLuint ubo, size_t offset, size_t length) {
            bindBufferRange(GL_UNIFORM_BUFFER, ubo, offset, length);
        }

        void bindSSBO(GLuint ssbo) {
            bindSSBORange(ssbo, 0, m_size);
        }

        void bindSSBORange(GLuint ssbo, size_t offset, size_t length) {
            bindBufferRange(GL_SHADER_STORAGE_BUFFER, ssbo, offset, length);
        }

        void bindAtomicCounter(GLuint atomic) {
            bindBufferRange(GL_ATOMIC_COUNTER_BUFFER, atomic, 0, m_size);
        }

        void bindBufferRange(GLenum target, GLuint binding, size_t offset, size_t length) {
            if (!m_created) return;

            if (m_binding != binding) {
                m_binding = binding;

                KI_DEBUG(fmt::format(
                    "BUFFER: bindBufferRange - name={}, id={}, size={}, binding={}, offset={}, len={}",
                    m_name, m_id, m_size, m_binding, offset, length));
            }

            glBindBufferRange(target, binding, m_id, offset, length);
        }

        void bindParameter() {
            if (!m_created) return;
            glBindBuffer(GL_PARAMETER_BUFFER, m_id);
        }

        void bindDrawIndirect() {
            if (!m_created) return;
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_id);
        }

        void bindVBO(GLuint vao, GLuint binding, size_t strideSize) {
            glVertexArrayVertexBuffer(vao, binding, m_id, 0, (GLsizei)strideSize);
        }

        // https://www.cppstories.com/2015/01/persistent-mapped-buffers-in-opengl/
        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml
        unsigned char* map(GLuint flags) {
            //return (unsigned char*)glMapNamedBuffer(m_id, flags);
            return mapRange(0, m_size, flags);
        }

        unsigned char* __restrict mapRange(size_t offset, size_t length, GLuint flags) {
            if (!m_created || m_mapped) return m_mappedData;

            KI_DEBUG(fmt::format(
                "BUFFER: mapRange - name={}, id={}, size={}, offset={}, len={}, flags={}",
                m_name, m_id, m_size, offset, length, flags));

            m_mappedData = (unsigned char*)glMapNamedBufferRange(m_id, offset, length, flags);
            m_mapped = true;
            m_mappedFlags = flags;
            return m_mappedData;
        }

        template <typename T>
        T* __restrict mapped(size_t offset) const
        {
            return (T*)(m_mappedData + offset);
        }

        void flushRange(size_t offset, size_t length) const
        {
            assert(m_mapped);
            assert(offset >= 0);
            assert(length > 0 && length <= m_size);

            // NOTE KI flush only for explicit
            // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glFlushMappedBufferRange.xhtml
            if (!(m_mappedFlags & GL_MAP_FLUSH_EXPLICIT_BIT)) return;

            glFlushMappedNamedBufferRange(m_id, offset, length);
        }

        void invalidate() const
        {
            glInvalidateBufferData(m_id);
        }

        void invalidateRange(size_t offset, size_t length) {
            assert(offset >= 0);
            assert(length > 0 && length <= m_size);
            glInvalidateBufferSubData(m_id, offset, length);
        }

        void unmap() {
            if (!m_mapped) return;
            glUnmapNamedBuffer(m_id);
            m_mappedData = nullptr;
            m_mapped = false;
        }

    private:
        std::string m_name;

        GLuint m_id = 0;
        size_t m_size = 0;
        GLuint m_flags = 0;

        size_t m_usedSize = 0;

        int m_binding = -1;

        unsigned char* __restrict m_mappedData{ nullptr };

        bool m_created = false;
        bool m_mapped = false;
        GLuint m_mappedFlags = 0;
    };
}
