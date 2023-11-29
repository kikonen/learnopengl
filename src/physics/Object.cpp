#include "Object.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "model/Node.h"
#include "PhysicsEngine.h"

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

    void Object::create(
        PhysicsEngine* engine,
        Node* node)
    {
        const auto& scale = node->getScale();
        const float radius = scale.x * m_geom.size.x;

        if (m_body.type != BodyType::none) {
            m_bodyId = dBodyCreate(engine->m_worldId);

            const glm::vec3& p = node->getWorldPosition();
            dBodySetPosition(m_bodyId, p[0], p[1], p[2]);

            switch (m_body.type) {
            case BodyType::sphere:
                dMassSetSphere(&m_mass, m_body.density, radius);
                break;
            }

            dBodySetMass(m_bodyId, &m_mass);
        }

        switch (m_geom.type) {
        case GeomType::sphere: {
            m_geomId = dCreateSphere(engine->m_spaceId, radius);
            break;
        }
        case GeomType::plane: {
            auto& p = m_geom.plane;
            m_geomId = dCreatePlane(engine->m_spaceId, p.x, p.y, p.z, p.a);
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
    }

    void Object::updateToPhysics(
        PhysicsEngine* engine,
        Node* node)
    {
        // TODO KI update physics if explicit move of object
    }

    void Object::updateFromPhysics(
        PhysicsEngine* engine,
        Node* node)
    {
        // TODO KI update node based into physics
    }
}
