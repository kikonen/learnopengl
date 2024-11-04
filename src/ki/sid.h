#pragma once

#include <string>
#include <unordered_map>

#include <ki/size.h>

namespace ki
{
    struct StringID
    {
        ki::sid_t m_sid;

        // NULL ID
        StringID()
            : m_sid{ 0 }
        {}

        StringID(std::string_view s);

        StringID(const StringID& o)
            : m_sid{ o.m_sid }
        {}

        StringID(ki::sid_t sid)
            : m_sid{ sid }
        {}

        const std::string& getName() const noexcept;

        const std::string& str() const noexcept
        {
            return getName();
        }

        operator std::string() const noexcept;

        operator uint32_t() const noexcept {
            return m_sid;
        }

        operator double() const noexcept {
            return static_cast<double>(m_sid);
        }

        operator bool() const noexcept {
            return m_sid != 0;
        }

        bool empty() const noexcept
        {
            return m_sid == 0;
        }

        bool operator==(const ki::StringID& o) const
        {
            return m_sid == o.m_sid;
        }

        static uint32_t nextID();
    };
}

// @see https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
template <>
struct std::hash<ki::StringID>
{
    std::size_t operator()(const ki::StringID& k) const
    {
        using std::size_t;
        using std::hash;
        using std::string;

        return std::hash<int>()(k.m_sid);
    }
};

#define SID(x) ki::StringID{ x }
