#include "BatchRegistry.h"

namespace
{
}

namespace render {
    int16_t BatchRegistry::getMultiDrawIndex(const MultiDrawKey& multiDraw)
    {
        const auto& it = m_multiDrawIndeces.find(multiDraw);
        if (it != m_multiDrawIndeces.end()) return it->second;

        auto index = static_cast<int16_t>(m_multiDraws.size());
        m_multiDraws.push_back(multiDraw);
        m_multiDrawIndeces.insert({ multiDraw, index });

        return index;
    }

    int16_t BatchRegistry::getCommandIndex(const CommandKey& cmd)
    {
        const auto& it = m_commandIndeces.find(cmd);
        if (it != m_commandIndeces.end()) return it->second;

        auto index = static_cast<int16_t>(m_commands.size());
        m_commands.push_back(cmd);
        m_commandIndeces.insert({ cmd, index });

        return index;
    }
}
