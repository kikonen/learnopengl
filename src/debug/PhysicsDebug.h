#pragma once

#include <memory>
#include <vector>
#include <string>
#include <atomic>

#include <glm/glm.hpp>

#include "ki/size.h"

namespace mesh {
    struct MeshInstance;
}

namespace debug {
    struct PhysicsDebug {
        bool m_updateEnabled{ true };
        bool m_showObjects{ false };

        bool m_dContactMu2{ false };
        bool m_dContactSlip1{ false };
        bool m_dContactSlip2{ false };
        bool m_dContactRolling{ false };
        bool m_dContactMotion1{ false };
        bool m_dContactMotion2{ false };
        bool m_dContactMotionN{ false };
        bool m_dContactBounce{ false };
        bool m_dContactSoftCFM{ false };
        bool m_dContactSoftERP{ false };
        bool m_dContactApprox1{ false };
        bool m_dContactFDir1{ false };

        float m_mu{ 0.f };
        float m_mu2{ 0.f };
        float m_rho{ 0.f };
        float m_rho2{ 0.f };
        float m_rhoN{ 0.f };
        float m_slip1{ 0.f };
        float m_slip2{ 0.f };
        float m_bounce{ 0.f };
        float m_bounce_vel{ 0.f };
        float m_motion1{ 0.f };
        float m_motion2{ 0.f };
        float m_motionN{ 0.f };
        float m_soft_erp{ 0.f };
        float m_soft_cfm{ 0.f };

        std::atomic<std::shared_ptr<std::vector<mesh::MeshInstance>>> m_meshesWT;
        std::atomic<std::shared_ptr<std::vector<mesh::MeshInstance>>> m_meshesPending;
        std::atomic<std::shared_ptr<std::vector<mesh::MeshInstance>>> m_meshesRT;

        void prepare();
    };
}
