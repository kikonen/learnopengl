#pragma once

#include <mutex>

template <typename T>
class IdGenerator {
public:
    T nextId() noexcept
    {
        std::lock_guard<std::mutex> lock(m_lock);
        return m_base++;
    }

private:
    T m_base{ 1 };

    std::mutex m_lock;
};
