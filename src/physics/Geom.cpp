#include "Geom.h"

#include "util/debug.h"

#include "component/definition/PhysicsDefinition.h"

#include "Body.h"

namespace physics {
    Geom::Geom() {}

    Geom::Geom(Geom&& o) noexcept
        : size{ o.size },
        rotation{ o.rotation },
        offset{ o.offset },
        physicId{ o.physicId },
        heightDataId{ o.heightDataId },
        categoryMask{ o.categoryMask },
        collisionMask{ o.collisionMask },
        materialId{ o.materialId },
        type{ o.type },
        placeable{ o.placeable }
    {
        // NOTE KI o is moved now
        o.physicId = nullptr;
        o.heightDataId = nullptr;
    }

    Geom::~Geom()
    {
        release();
    }

    Geom& Geom::operator=(Geom&& o) noexcept
    {
        release();

        size = o.size;
        rotation = o.rotation;
        offset = o.offset;
        physicId = o.physicId;
        heightDataId = o.heightDataId;
        categoryMask = o.categoryMask;
        collisionMask = o.collisionMask;
        materialId = o.materialId;
        type = o.type;
        placeable = o.placeable;

        // NOTE KI o is moved now
        o.physicId = nullptr;
        o.heightDataId = nullptr;

        return *this;
    }

    Geom& Geom::operator=(const GeomDefinition& o)
    {
        release();

        size = o.m_size;
        rotation = o.m_rotation;
        offset = o.m_offset;
        categoryMask = o.m_categoryMask;
        collisionMask = o.m_collisionMask;
        type = o.m_type;
        placeable = o.m_placeable;

        return *this;
    }

    void Geom::release()
    {
        if (physicId) {
            dGeomDestroy(physicId);
            physicId = nullptr;
        }
        if (heightDataId) {
            dGeomHeightfieldDataDestroy(heightDataId);
            heightDataId = nullptr;
        }
    }

    void Geom::create(
        physics::object_id objectId,
        dWorldID worldId,
        dSpaceID spaceId,
        const glm::vec3& scale,
        dBodyID bodyPhysicId)
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
        case GeomType::height_field: {
            heightDataId = dGeomHeightfieldDataCreate();
            // NOTE KI placeable to allow setting origin
            physicId = dCreateHeightfield(spaceId, heightDataId, placeable);
            break;
        }
        case GeomType::box: {
            physicId = dCreateBox(spaceId, sz.x * 2.f, sz.y * 2.f, sz.z * 2.f);
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
        if (bodyPhysicId) {
            const auto& so = scale * offset;
            dGeomSetBody(physicId, bodyPhysicId);
            dGeomSetOffsetPosition(physicId, so.x, so.y, so.z);
        }

        {
            //KI_INFO_OUT(fmt::format("GEOM: cat={}, col={}", categoryMask, collisionMask));
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

        dGeomSetData(physicId, (void*)objectId);
    }

    void Geom::updatePhysic(
        const glm::vec3& nodePivot,
        const glm::vec3& nodePos,
        const glm::quat& nodeRot) const
    {
        if (type == GeomType::none) return;

        // NOTE KI handle bodiless geoms separately
        //const auto& sz = size;
        //const float radius = sz.x;
        //const float length = sz.y * 2.f;

        bool handled = false;
        switch (type) {
        case GeomType::plane: {
            setPlane(nodePivot, nodeRot * rotation);
            handled = true;
            break;
        }
        case GeomType::height_field: {
            setHeightField(nodePivot, nodeRot * rotation);
            handled = true;
            break;
        }
        }

        if (!handled) {
            setPhysicPosition(nodePos);
            setPhysicRotation(nodeRot);
        }
    }

    void Geom::setPlane(const glm::vec3& pos, const glm::quat& rot) const
    {
        const glm::vec3 UP{ 0.f, 1.f, 0.f };

        auto normal = rot * UP;

        // NOTE KI distance into direction of plane normal
        auto dist = glm::dot(normal, pos);

        dGeomPlaneSetParams(physicId, normal.x, normal.y, normal.z, dist);
    }

    void Geom::setHeightField(const glm::vec3& pos, const glm::quat& rot) const
    {
        if (placeable) {
            // HACK KI match current terrain placement logic
            // => would be better to change terrain to use "center point"?!?
            setPhysicPosition(pos + size * 0.5f);
            setPhysicRotation(rot);
        }
    }
}
