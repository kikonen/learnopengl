#pragma once

#include <string>
#include <vector>

#include "ki/size.h"

namespace animation
{
/*
    - serializedVersion: 16
      name: Hit
      takeName: Take 001
      internalID: 0
      firstFrame: 48
      lastFrame: 98
      wrapMode: 0
      orientationOffsetY: 0
      level: 0
      cycleOffset: 0
      loop: 0
      hasAdditiveReferencePose: 0
      loopTime: 0
      loopBlend: 0
      loopBlendOrientation: 0
      loopBlendPositionY: 0
      loopBlendPositionXZ: 0
      keepOriginalOrientation: 0
      keepOriginalPositionY: 1
      keepOriginalPositionXZ: 0
      heightFromFeet: 0
      mirror: 0
      bodyMask: 01000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000
      curves: []
      events: []
      transformMask:
        - path:
          weight: 1
        - path: Bip01
          weight: 1
        ...
      maskType: 0
      maskSource: {instanceID: 0}
      additiveReferencePoseFrame: 0
    - serializedVersion: 16
      name: Walk01
      takeName: Take 001
      internalID: 0
      firstFrame: 1143
      lastFrame: 1175
      wrapMode: 0
      orientationOffsetY: 0
      level: 0
      cycleOffset: 0
      loop: 0
      hasAdditiveReferencePose: 0
      loopTime: 0
      loopBlend: 0
      loopBlendOrientation: 0
      loopBlendPositionY: 0
      loopBlendPositionXZ: 0
      keepOriginalOrientation: 0
      keepOriginalPositionY: 1
      keepOriginalPositionXZ: 0
      heightFromFeet: 0
      mirror: 0
      bodyMask: 01000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000
      curves: []
      events: []
      transformMask:
      - path:
        weight: 1
      maskType: 0
      maskSource: {instanceID: 0}
      additiveReferencePoseFrame: 0
*/
    struct Clip {
        std::string m_name;
        ki::sid_t m_id{ 0 };
        int32_t m_index{ -1 };

        std::string m_animationName;
        int32_t m_animationIndex{ -1 };

        uint16_t m_firstFrame{ 0 };
        uint16_t m_lastFrame{ 0 };

        float m_duration{ 0 };
        float m_durationSecs{ 0 };

        bool m_loop{ false };

        bool m_single{ true };
        //std::vector<std::string> m_events;
    };
}
