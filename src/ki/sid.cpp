#include "sid.h"

#include <unordered_map>
#include <mutex>

#include <fmt/format.h>

#include "pool/IdHash.h"

namespace {
    std::mutex g_lock;

    constexpr uint32_t NULL_ID{ 0 };
    const std::string MISSING = "";

    uint32_t g_idSeq{ 0 };

    std::unordered_map<uint32_t, std::string> g_registeredIds;

    void registerName(uint32_t sid, const std::string& s)
    {
        std::lock_guard lock{ g_lock };
        const auto& it = g_registeredIds.find(sid);
        if (it != g_registeredIds.end()) return;
        g_registeredIds.insert({ sid, s });
    }

    const std::string& findName(uint32_t sid)
    {
        std::lock_guard lock{ g_lock };
        const auto& it = g_registeredIds.find(sid);
        return it != g_registeredIds.end() ? it->second : MISSING;
    }

    uint32_t seqNext()
    {
        std::lock_guard lock{ g_lock };
        return ++g_idSeq;
    }
}

StringID::StringID(std::string_view s)
    : m_sid{ s.empty() ? NULL_ID : pool::IdHash::make32(s) }
{
    registerName(*this, std::string{ s });
}

const std::string& StringID::getName() const noexcept
{
    return findName(*this);
}

StringID::operator std::string() const noexcept
{
    return fmt::format("[SID:{}/{}]", m_sid, getName());
}

uint32_t StringID::nextID()
{
    StringID sid{ fmt::format("auto-{}", seqNext()) };
    return sid;
}
