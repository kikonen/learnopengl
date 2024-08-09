#pragma once

#include <string>

struct StringID
{
    uint32_t m_sid;

    StringID(std::string_view s);

    StringID(const StringID& o)
        : m_sid{ o.m_sid }
    {}

    const std::string& getName() const noexcept;

    operator std::string() const noexcept;

    operator uint32_t() const noexcept {
        return m_sid;
    }

    operator double() const noexcept {
        return static_cast<double>(m_sid);
    }
};

#define SID(x) StringID{ x }
