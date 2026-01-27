#include "AnimationDebug.h"

#include "asset/Assets.h"

namespace debug
{
    void AnimationDebug::prepare()
    {
        const auto& assets = Assets::get();

        m_showSockets = assets.animationShowSockets;
    }
}
