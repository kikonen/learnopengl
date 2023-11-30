#include "Object.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "model/Node.h"
#include "PhysicsEngine.h"

namespace {
    constexpr int DIR_X = 1;
    constexpr int DIR_Y = 2;
    constexpr int DIR_Z = 3;
}

namespace physics
{
    Object::~Object()
    {
        if (m_geomId) {
            dGeomDestroy(m_geomId);
        }
        if (m_bodyId) {
            dBodyDestroy(m_bodyId);
        }
    }

    void Object::prepare(
        PhysicsEngine* engine,
        Node* node)
    {
        if (ready()) return;

        const auto& sz = m_geom.size;
        const float radius = sz.x;
        const float length = sz.y;

        if (m_body.type != BodyType::none) {
            m_bodyId = dBodyCreate(engine->m_worldId);

            switch (m_body.type) {
            case BodyType::box: {
                dMassSetBox(&m_mass, m_body.density, sz.x, sz.y, sz.z);
                break;
            }
            case BodyType::sphere: {
                dMassSetSphere(&m_mass, m_body.density, radius);
                break;
            }
            case BodyType::capsule: {
                dMassSetCapsule(&m_mass, m_body.density, DIR_Z, radius, length);
                break;
            }
            case BodyType::cylinder: {
                dMassSetCylinder(&m_mass, m_body.density, DIR_Z, radius, length);
                break;
            }
            }

            dBodySetMass(m_bodyId, &m_mass);
        }

        switch (m_geom.type) {
        case GeomType::plane: {
            const auto& plane = m_geom.plane;
            m_geomId = dCreatePlane(engine->m_spaceId, plane.x, plane.y, plane.z, plane.a);
            break;
        }
        case GeomType::box: {
            m_geomId = dCreateBox(engine->m_spaceId, sz.x, sz.y, sz.z);
            break;
        }
        case GeomType::sphere: {
            m_geomId = dCreateSphere(engine->m_spaceId, radius);
            break;
        }
        case GeomType::capsule: {
            m_geomId = dCreateCapsule(engine->m_spaceId, radius, length);
            break;
        }
        case GeomType::cylinder: {
            const float radius = m_geom.size.x;
            const float length = m_geom.size.y;
            m_geomId = dCreateCylinder(engine->m_spaceId, radius, length);
            break;
        }
        }

        if (m_geomId) {
            // NOTE KI node updates only if body
            if (m_bodyId) {
                dGeomSetBody(m_geomId, m_bodyId);
            }
            m_node = node;
            engine->registerObject(this);
        }
        updateToPhysics(true);
    }

    void Object::updateToPhysics(bool force)
    {
        if (!(force || m_update)) return;

        const glm::vec3& pos = m_node->getWorldPosition();

        if (m_bodyId) {
            dBodySetPosition(m_bodyId, pos[0], pos[1], pos[2]);
            if (m_body.kinematic) {
                dBodySetKinematic(m_bodyId);
            }
            else {
                dBodySetDynamic(m_bodyId);
            }
        }

        if (m_geomId) {
            const auto& scale = m_node->getScale();

            const auto& size = m_geom.size;
            const auto& sz = scale * m_geom.size;
            const float radius = sz.x;
            const float length = sz.y;

            switch (m_geom.type) {
            case GeomType::plane: {
                const auto& plane = m_geom.plane;
                dGeomPlaneSetParams(m_geomId, plane.x, plane.y, plane.z, pos.y);
                break;
            }
            case GeomType::box: {
                dGeomBoxSetLengths(m_geomId, sz.x, sz.y, sz.z);
                break;
            }
            case GeomType::sphere: {
                dGeomSphereSetRadius(m_geomId, radius);
                break;
            }
            case GeomType::capsule: {
                dGeomCapsuleSetParams(m_geomId, radius, length);
                break;
            }
            case GeomType::cylinder: {
                dGeomCylinderSetParams(m_geomId, radius, length);
                break;
            }
            }
        }
    }

    void Object::updateFromPhysics()
    {
        if (!m_bodyId) return;

        const dReal* dpos = dBodyGetPosition(m_bodyId);

        glm::vec3 pos = { dpos[0], dpos[1], dpos[2] };
        if (pos.y < -40) {
            pos.y = -40;
            dBodySetPosition(m_bodyId, pos[0], pos[1], pos[2]);
        }
        if (pos.y > 40) {
            pos.y = 40;
            dBodySetPosition(m_bodyId, pos[0], pos[1], pos[2]);
        }
        pos -= m_node->getParent()->getWorldPosition();
        m_node->setPosition(pos);
    }
}
