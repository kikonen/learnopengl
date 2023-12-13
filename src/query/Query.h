#pragma once

#include "kigl/kigl.h"

class Query {
public:
    Query(GLenum target)
        : m_target{ target }
    {}

    virtual ~Query() {
        if (m_id <= 0) return;

        if (m_started) glEndQuery(m_target);
        glDeleteQueries(1, &m_id);
    }

    Query(Query& handle) = delete;
    Query& operator=(Query& handle) = delete;

    Query(Query&& handle) noexcept
        : m_target(handle.m_target),
        m_id(handle.m_id)
    {
        handle.m_id = 0;
    }

    Query& operator=(Query&& handle) noexcept
    {
        m_target = handle.m_target;
        m_id = handle.m_id;
        handle.m_id = 0;
        return *this;
    }

    bool valid() { return m_id > 0; }

    virtual void create()
    {
        glCreateQueries(m_target, 1, &m_id);
    }

    virtual void reset() {
        m_count = 0;
    }

    void begin()
    {
        if (m_started) return;
        m_started = true;

        // NOTE KI starting new query without fetching result messes up result
        if (m_needResult) fetchResult();

        glBeginQuery(m_target, m_id);
    }

    void end()
    {
        if (!m_started) return;
        m_started = false;
        m_needResult = true;
        m_count++;

        glEndQuery(m_target);
    }

    // NOTE KI result *can* and *will* be delayed
    virtual void fetchResult()
    {
        m_needResult = false;
    }

    unsigned long count() { return m_count; }

    operator int() const { return m_id; }

protected:
    GLenum m_target;

    GLuint m_id{ 0 };
    bool m_started{ false };
    bool m_needResult{ false };

    unsigned long m_count{ 0 };
};
