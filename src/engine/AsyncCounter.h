#pragma once

#include <atomic>

struct AsyncAdd
{
    AsyncAdd(
        std::atomic<int>& out, int v)
        : m_out{ out },
        m_value{ v }
    {
    }

    ~AsyncAdd()
    {
        m_out.fetch_add(m_value);
    }

private:
    std::atomic<int>& m_out;
    int m_value;
};
