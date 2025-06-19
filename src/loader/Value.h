#pragma once

#include <optional>

namespace loader
{
    template <class T>
    class Value final
    {
    public:
        Value(Args... defaultValue)
        {
            m_defaultValue = defaultValue;
        }

        std::optional<T> m_defaultValue;
        std::optional<T> m_value;

        operator=(const T& value)
        {
            m_value = value;
        }

        operator T&() const
        {
            if (m_value.has_value()) return *m_value;
            *m_defaultValue;
        }

        const T& operator*(void) const
        {
            if (m_value.has_value()) return *m_value;
            return *m_defaultValue;
        }

        //T* operator->(void)
        //{
        //    if (!m_value.has_value()) {
        //        m_value = T{};
        //    }
        //    return *m_value;
        //}

        T& operator()(void)
        {
            if (m_value.has_value()) {
                m_value = T{};
            }

            return m_value;
        }
    };
}
