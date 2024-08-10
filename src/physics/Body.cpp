#include "Body.h"

#include "util/glm_util.h"

namespace {
    constexpr int DIR_X = 1;
    constexpr int DIR_Y = 2;
    constexpr int DIR_Z = 3;

    //data.baseRotation = glm::quat_identity<float, glm::packed_highp>();
    const glm::quat IDENTITY_QUAT{ 0.f, 0.f, 0.f, 0.f };
}

namespace physics {
    Body::~Body()
    {
        if (physicId) {
            dBodyDestroy(physicId);
        }
    }

    void Body::create(
        dWorldID worldId,
        dSpaceID spaceId,
        const glm::vec3& scale)
    {
        invBaseRotation = glm::conjugate(baseRotation);

        if (type == BodyType::none) return;

        physicId = dBodyCreate(worldId);

        auto sz = scale * size;
        float radius = sz.x;
        float length = sz.y * 2.f;

        // Mass
        {
            dMass mass;

            switch (type) {
            case BodyType::box: {
                dMassSetBox(&mass, density, sz.x, sz.y, sz.z);
                break;
            }
            case BodyType::sphere: {
                dMassSetSphere(&mass, density, radius);
                break;
            }
            case BodyType::capsule: {
                dMassSetCapsule(&mass, density, DIR_Z, radius, length);
                break;
            }
            case BodyType::cylinder: {
                dMassSetCylinder(&mass, density, DIR_Z, radius, length);
                break;
            }
            }
            dBodySetMass(physicId, &mass);
        }

        // Static base setup
        {
            if (kinematic) {
                dBodySetKinematic(physicId);
            }
            else {
                dBodySetDynamic(physicId);
            }
            dBodySetMaxAngularSpeed(physicId, maxAngulerVelocity);

            //// TODO KI updateToPhysics() *WILL* override this
            //if (const auto& q = baseRotation;
            //    q != IDENTITY_QUAT)
            //{
            //    dQuaternion quat{ q.w, q.x, q.y, q.z };
            //    dBodySetQuaternion(physicId, quat);

            //    //const dReal* qp = dBodyGetQuaternion(physicId);
            //    //glm::quat quat2{
            //    //    static_cast<float>(qp[0]),
            //    //    static_cast<float>(qp[1]),
            //    //    static_cast<float>(qp[2]),
            //    //    static_cast<float>(qp[3]) };
            //    //auto deg = util::quatToDegrees(quat2);
            //    //int x = 0;
            //}
        }
    }
}
