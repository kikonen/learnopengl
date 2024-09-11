#pragma once

#include "ode/ode.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "size.h"

namespace physics
{
    void setPhysicPosition(dGeomID geomID, const glm::vec3& pos)
    {
        dGeomSetPosition(geomID, pos.x, pos.y, pos.z);
    }

    // NOTE KI  If the geom is attached to a body,
    // the body's position / rotation pointers will be returned, i.e. the result
    // will be identical to calling dBodyGetPosition or dBodyGetRotation.
    glm::vec3 getPhysicPosition(dGeomID geomId)
    {
        const dReal* dpos = dGeomGetPosition(geomId);
        return {
            static_cast<float>(dpos[0]),
            static_cast<float>(dpos[1]),
            static_cast<float>(dpos[2]) };
    }

    void setPhysicRotation(dGeomID geomId, const glm::quat& rot)
    {
        dQuaternion dquat{ rot.w, rot.x, rot.y, rot.z };
        dGeomSetQuaternion(geomId, dquat);
    }

    // NOTE KI  If the geom is attached to a body,
    // the body's position / rotation pointers will be returned, i.e. the result
    // will be identical to calling dBodyGetPosition or dBodyGetRotation.
    glm::quat getPhysicRotation(dGeomID geomId)
    {
        dQuaternion dquat;
        dGeomGetQuaternion(geomId, dquat);

        return {
            static_cast<float>(dquat[0]),
            static_cast<float>(dquat[1]),
            static_cast<float>(dquat[2]),
            static_cast<float>(dquat[3]) };
    }

    physics::GeomType getGeomType(int geomClass)
    {
        switch (geomClass) {
        case dRayClass:
            return physics::GeomType::ray;
        case dPlaneClass:
            return physics::GeomType::plane;
        case dHeightfieldClass:
            return physics::GeomType::height_field;
        case dBoxClass:
            return physics::GeomType::box;
        case dSphereClass:
            return physics::GeomType::sphere;
        case dCapsuleClass:
            return physics::GeomType::capsule;
        case dCylinderClass:
            return physics::GeomType::cylinder;
        }
        return physics::GeomType::none;
    }
}
