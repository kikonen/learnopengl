#pragma once

#include <vector>
#include <unordered_map>

#include "ki/size.h"

#include "BatchCommand.h"

//return m_renderBack == o.m_renderBack &&
//(forceLineMode ? true : m_lineMode == o.m_lineMode) &&
//(forceSolid ? true : m_blend == o.m_blend) &&
//m_reverseFrontFace == o.m_reverseFrontFace &&
//m_noDepth == o.m_noDepth &&
//m_clip == o.m_clip &&
//m_mode == o.m_mode &&
//m_type == o.m_type;

template <>
struct std::hash<backend::DrawOptions>
{
    size_t operator()(const backend::DrawOptions& k) const
    {
        // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
        return (std::hash<backend::DrawOptions::Mode>()(k.m_mode) << 1)
            ^ (std::hash<backend::DrawOptions::Type>()(k.m_type) << 1)
            ^ ((std::hash<int>()(k.m_renderBack)
            ^ (std::hash<int>()(k.m_lineMode) << 1)
            ^ (std::hash<int>()(k.m_kindBits) << 1)
            ^ (std::hash<int>()(k.m_reverseFrontFace) << 1)
            ^ (std::hash<int>()(k.m_noDepth) << 1)
            ) >> 1);
    }
};

template <>
struct std::hash<render::CommandKey>
{
    size_t operator()(const render::CommandKey& k) const
    {
        // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
        return ((std::hash<int>()(k.m_baseVertex)
            ^ (std::hash<int>()(k.m_baseIndex) << 1)
            ) >> 1);
    }
};

template <>
struct std::hash<render::MultiDrawKey>
{
    size_t operator()(const render::MultiDrawKey& k) const
    {
        // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
        return ((std::hash<int>()(k.m_vaoId)
            ^ (std::hash<int>()(k.m_programId) << 1)
            ^ (std::hash<backend::DrawOptions>()(k.m_drawOptions))
            ) >> 1);
    }
};

namespace render {
    // Track unique indexes for multidraws and draw mesh commands
    class BatchRegistry {
    public:
        // index, 0 == null
        int16_t getMultiDrawIndex(const MultiDrawKey& multiDraw);

        // index, 0 == null
        int16_t getCommandIndex(const CommandKey& cmd);

        inline const MultiDrawKey* getMultiDraw(int16_t index) const noexcept
        {
            return &m_multiDraws[index];
        }

        // index, 0 == null
        const CommandKey* getCommand(int16_t index) const noexcept
        {
            return &m_commands[index];
        }

        size_t getMaxMultDrawIndex() const noexcept
        {
            return m_multiDraws.size();
        }

        size_t getMaxCommandIndex() const noexcept
        {
            return m_commands.size();
        }

    private:
        std::vector<MultiDrawKey> m_multiDraws;
        std::vector<CommandKey> m_commands;

        std::unordered_map<MultiDrawKey, int16_t> m_multiDrawIndeces;
        std::unordered_map<CommandKey, int16_t> m_commandIndeces;
    };
}
