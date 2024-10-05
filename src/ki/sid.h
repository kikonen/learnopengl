#pragma once

#include <string>

#include <ki/size.h>

struct StringID
{
    ki::sid m_sid;

    // NULL ID
    StringID()
        : m_sid{ 0 }
    {}

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

    static uint32_t nextID();
};

#define SID(x) StringID{ x }
