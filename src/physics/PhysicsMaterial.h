#pragma once

#include <string>

#include "size.h"

namespace physics {
    struct PhysicsMaterial {
        PhysicsMaterial();

        physics::material_id m_id{ 0 };

        bool physics_dContactMu2;
        bool physics_dContactSlip1;
        bool physics_dContactSlip2;
        bool physics_dContactRolling;
        bool physics_dContactBounce;
        bool physics_dContactMotion1;
        bool physics_dContactMotion2;
        bool physics_dContactMotionN;
        bool physics_dContactSoftCFM;
        bool physics_dContactSoftERP;
        bool physics_dContactApprox1;
        bool physics_dContactFDir1;

        float physics_mu;
        float physics_mu2;
        float physics_rho;
        float physics_rho2;
        float physics_rhoN;
        float physics_slip1;
        float physics_slip2;
        float physics_bounce;
        float physics_bounce_vel;
        float physics_motion1;
        float physics_motion2;
        float physics_motionN;
        float physics_soft_erp;
        float physics_soft_cfm;
    };
}
