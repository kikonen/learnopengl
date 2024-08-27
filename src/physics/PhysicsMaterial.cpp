#include "PhysicsMaterial.h"

#include "ode/ode.h"

namespace physics {
    PhysicsMaterial::PhysicsMaterial()
    {
        physics_dContactMu2 = true;
        physics_dContactSlip1 = true;
        physics_dContactSlip2 = true;
        physics_dContactRolling = false;
        physics_dContactBounce = true;
        physics_dContactMotion1 = false;
        physics_dContactMotion2 = false;
        physics_dContactMotionN = false;
        physics_dContactSoftCFM = true;
        physics_dContactSoftERP = true;
        physics_dContactApprox1 = true;
        physics_dContactFDir1 = false;

        physics_mu = 100;// dInfinity;
        physics_mu2 = 0;
        physics_rho = 0;
        physics_rho2 = 0;
        physics_rhoN = 0;
        physics_slip1 = 0.7;
        physics_slip2 = 0.7;
        physics_bounce = 0.6;
        physics_bounce_vel = 1.1;
        physics_motion1 = 0;
        physics_motion2 = 0;
        physics_motionN = 0;
        physics_soft_erp = 0.9;
        physics_soft_cfm = 0.9;
    };
}
