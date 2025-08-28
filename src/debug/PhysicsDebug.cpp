#include "PhysicsDebug.h"

#include "asset/Assets.h"

namespace debug {

    void PhysicsDebug::prepare()
    {
        const auto& assets = Assets::get();
        auto& dbg = *this;

        dbg.m_updateEnabled = assets.physicsUpdateEnabled;
        dbg.m_showObjects = assets.physicsShowObjects;

        dbg.m_dContactMu2 = assets.physics_dContactMu2;
        dbg.m_dContactSlip1 = assets.physics_dContactSlip1;
        dbg.m_dContactSlip2 = assets.physics_dContactSlip2;
        dbg.m_dContactRolling = assets.physics_dContactRolling;
        dbg.m_dContactBounce = assets.physics_dContactBounce;
        dbg.m_dContactMotion1 = assets.physics_dContactMotion1;
        dbg.m_dContactMotion2 = assets.physics_dContactMotion2;
        dbg.m_dContactMotionN = assets.physics_dContactMotionN;
        dbg.m_dContactSoftERP = assets.physics_dContactSoftERP;
        dbg.m_dContactSoftCFM = assets.physics_dContactSoftCFM;
        dbg.m_dContactApprox1 = assets.physics_dContactApprox1;
        dbg.m_dContactFDir1 = assets.physics_dContactFDir1;

        dbg.m_mu = assets.physics_mu;
        dbg.m_mu2 = assets.physics_mu2;
        dbg.m_rho = assets.physics_rho;
        dbg.m_rho2 = assets.physics_rho2;
        dbg.m_rhoN = assets.physics_rhoN;
        dbg.m_slip1 = assets.physics_slip1;
        dbg.m_slip2 = assets.physics_slip2;
        dbg.m_bounce = assets.physics_bounce;
        dbg.m_bounce_vel = assets.physics_bounce_vel;
        dbg.m_motion1 = assets.physics_motion1;
        dbg.m_motion2 = assets.physics_motion2;
        dbg.m_motionN = assets.physics_motionN;
        dbg.m_soft_erp = assets.physics_soft_erp;
        dbg.m_soft_cfm = assets.physics_soft_cfm;
    }
}
