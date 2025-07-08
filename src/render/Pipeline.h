#pragma once

namespace render
{
    struct Pipeline
    {
        bool m_deferred : 1 { true };
        bool m_preDepth : 1 { true };
        bool m_forward : 1 { true };
        bool m_decal : 1 { true };
        bool m_particle : 1 { true };
        bool m_effect : 1 { true };
        bool m_fog : 1 { true };
        bool m_oit : 1 { true };
        bool m_ssao : 1 { true };
        bool m_emission : 1 { true };
        bool m_bloom : 1 { true };
        bool m_skybox : 1 { true };
        bool m_debug : 1 { false };
        bool m_debugPhysics : 1 { false };
        bool m_debugVolume : 1 { false };
        bool m_debugEnvironmentProbe : 1 { false };
        bool m_debugNormal : 1 { false };
        bool m_copy : 1 { true };
    };
}
