#include "Geom.h"

#include "util/debug.h"

#include "Body.h"

namespace physics {
    Geom::~Geom()
    {
        if (physicId) {
            dGeomDestroy(physicId);
        }
    }

    void Geom::create(
        dWorldID worldId,
        dSpaceID spaceId,
        const glm::vec3& scale,
        const Body& body)
    {
        if (type == GeomType::none) return;

        auto sz = scale * size;
        float radius = sz.x;
        float length = sz.y * 2.f;

        switch (type) {
        case GeomType::ray: {
            physicId = dCreateRay(spaceId, 0);
            dGeomRaySet(physicId, 0, 0, 0, 0, 0, 1);
            break;
        }
        case GeomType::plane: {
            // TODO KI updateToPhysics() *WILL* override this
            glm::vec3 normal{ 0, 1.f, 0 };
            float dist = 0.f;
            auto plane = rotation * glm::vec4(normal, 1.f);

            physicId = dCreatePlane(spaceId, plane.x, plane.y, plane.z, dist);
            break;
        }
        case GeomType::box: {
            physicId = dCreateBox(spaceId, sz.x, sz.y, sz.z);
            break;
        }
        case GeomType::sphere: {
            physicId = dCreateSphere(spaceId, radius);
            break;
        }
        case GeomType::capsule: {
            physicId = dCreateCapsule(spaceId, radius, length);
            break;
        }
        case GeomType::cylinder: {
            physicId = dCreateCylinder(spaceId, radius, length);
            break;
        }
        }

        if (!physicId) return;

        // NOTE KI node updates only if body
        if (body.physicId) {
            const auto& so = scale * offset;
            dGeomSetBody(physicId, body.physicId);
            dGeomSetOffsetPosition(physicId, so.x, so.y, so.z);
        }

        {
            KI_INFO_OUT(fmt::format("cat={}, col={}", categoryMask, collisionMask));
            dGeomSetCategoryBits(physicId, categoryMask);
            dGeomSetCollideBits(physicId, collisionMask);

            //if (const auto& q = m_geom.quat;
            //    rotation != NULL_QUAT)
            //{
            //    dQuaternion quat{ q.w, q.x, q.y, q.z };
            //    dGeomSetQuaternion(physicId, quat);

            //    dQuaternion quat2;
            //    dGeomGetQuaternion(physicId, quat2);
            //    int x = 0;
            //}
        }
    }
}
