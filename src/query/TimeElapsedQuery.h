#pragma once

#include "Query.h"

class TimeElapsedQuery : public Query {
public:
    TimeElapsedQuery()
        : Query(GL_TIME_ELAPSED)
    {}

    virtual void reset() override {
        Query::reset();
        m_elapsedTotal = 0;
    }

    virtual void fetchResult() override
    {
        if (!m_needResult) return;
        m_needResult = false;

        GLuint64 elapsed = 0;
        glGetQueryObjectui64v(m_id, GL_QUERY_RESULT, &elapsed);
        m_elapsedTotal += elapsed;
    }

    unsigned long elapsed(bool block) {
        if (block) fetchResult();
        return m_elapsedTotal;
    }

    double avg(bool block) {
        if (block) fetchResult();
        return m_elapsedTotal / (double)m_count;
    }

private:
    // will be in nanoseconds
    unsigned long m_elapsedTotal{ 0 };
};
