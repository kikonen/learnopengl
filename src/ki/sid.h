#pragma once

#include <string>

#include <ki/size.h>

namespace ki
{
    struct StringID
    {
        static void registerSystemId(ki::sid_t sid, const std::string& name) noexcept;

        static const std::string& getName(ki::sid_t sid) noexcept;
        static const std::string& getName(ki::StringID sid) noexcept;

        ki::sid_t m_sid{ 0 };

        // NULL ID
        StringID() noexcept = default;

        StringID(const StringID&) noexcept = default;
        StringID(StringID&&) noexcept = default;
        StringID& operator=(const StringID&) noexcept = default;
        StringID& operator=(StringID&&) noexcept = default;
        ~StringID() = default;

        StringID(ki::sid_t sid) noexcept : m_sid{ sid } {}
        StringID(std::string_view s, bool add);

        const std::string& getName() const noexcept;

        const std::string& str() const noexcept
        {
            return getName();
        }

        operator std::string() const noexcept;

        ki::sid_t asSid() const noexcept
        {
            return m_sid;
        }

        uint32_t asInt() const noexcept
        {
            return m_sid;
        }

        bool asBool() const noexcept
        {
            return m_sid != 0;
        }

        double asDouble() const noexcept
        {
            return static_cast<double>(m_sid);
        }

        explicit operator uint32_t() const noexcept {
            return m_sid;
        }

        explicit operator double() const noexcept {
            return static_cast<double>(m_sid);
        }

        explicit operator bool() const noexcept {
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

        static StringID nextID();
        static StringID nextID(std::string_view base);
    };

    static_assert(std::is_trivially_copyable_v<ki::StringID>);
    static_assert(sizeof(ki::StringID) == sizeof(ki::sid_t));
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

#define SID(x) ki::StringID{ x, false }
#define SID_REGISTER(x) ki::StringID{ x, true }
#define SID_NAME(x) ki::StringID::getName(x)
