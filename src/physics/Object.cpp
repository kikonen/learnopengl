#include "Object.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/Log.h"
#include "util/Util.h"
#include "util/glm_util.h"

#include "model/Node.h"

#include "registry/NodeRegistry.h"

namespace {
    inline glm::vec3 UP{ 0.f, 1.f, 0.f };

    const glm::quat IDENTITY_QUAT{ 0.f, 0.f, 0.f, 0.f };
}

namespace physics
{
    Object::Object() = default;

    //Object::Object(const Object& o) noexcept
    //    : m_body{ o.m_body },
    //    m_geom{ std::move(o.m_geom) }
    //{
    //    // NOTE KI cannot copy physics bindings
    //    m_body.physicId = nullptr;
    //    m_geom.physicId = nullptr;
    //    m_geom.heightDataId = nullptr;
    //}

    Object::Object(Object&& o) noexcept
        : m_body{ o.m_body },
        m_geom{ std::move(o.m_geom) }
    {
        // NOTE KI o is moved now
        o.m_body.physicId = nullptr;
        o.m_geom.physicId = nullptr;
        o.m_geom.heightDataId = nullptr;
    }

    Object::~Object() = default;

    void Object::create(
        uint32_t entityIndex,
        dWorldID worldId,
        dSpaceID spaceId,
        NodeRegistry& nodeRegistry)
    {
        if (ready()) return;

        glm::vec3 scale{ 1.f };

        // NOtE KI assumping that parent scale does not affect node
        // => 99,9% true for physics nodes, i.e. does not make sense
        // for them to have parent node affecting scale
        const auto& state = nodeRegistry.getState(entityIndex);
        scale = state.getScale();

        m_body.create(worldId, spaceId, scale);
        m_geom.create(worldId, spaceId, scale, m_body.physicId);
    }

    void Object::updateToPhysics(
        uint32_t entityIndex,
        ki::level_id& matrixLevel,
        NodeRegistry& nodeRegistry)
    {
        const auto& state = nodeRegistry.getState(entityIndex);
        {
            const auto level = state.getMatrixLevel();
            if (matrixLevel == level) return;
            matrixLevel = level;
        }

        const glm::vec3& pos = state.getWorldPosition();
        const glm::vec3& pivot = state.getWorldPivot();
        const auto& rot = state.getModelRotation() * m_body.baseRotation;

        if (m_body.physicId) {
            m_body.updatePhysic(pivot, pos, rot);

            //dBodySetLinearVel(m_body.physicId, 0.f, 0.f, 0.f);
            dBodySetAngularVel(m_body.physicId, 0.f, 0.f, 0.f);
            //dBodySetForce(m_body.physicId, 0.f, 0.f, 0.f);
            dBodySetTorque(m_body.physicId, 0.f, 0.f, 0.f);
        }
        else if (m_geom.physicId)
        {
            // NOTE KI for "geom only" nodes
            m_geom.updatePhysic(pivot, pos, rot);
        }
    }

    void Object::updateFromPhysics(
        uint32_t entityIndex,
        NodeRegistry& nodeRegistry) const
    {
        // NOTE KI "geom only" is not updated back to node
        if (!m_body.physicId) return;
        if (m_body.kinematic) return;

        auto& state = nodeRegistry.modifyState(entityIndex);
        auto& parentState = nodeRegistry.getParentState(entityIndex);

        glm::vec3 pos{ 0.f };
        glm::quat rot{ 1.f, 0.f, 0.f, 0.f };
        {
            pos = m_body.getPhysicPosition();
            rot = m_body.getPhysicRotation();

            // NOTE KI parent *SHOULD* be root (== null) for all physics nodes
            // => otherwise math does not make sense
            // => for NodeGenerator based nodes parent is generator container
            pos -= parentState.getWorldPosition();
            pos -= (state.getWorldPivot() - state.getWorldPosition());

            // https://danceswithcode.net/engineeringnotes/quaternions/quaternions.html
            rot = glm::normalize(rot * m_body.invBaseRotation);

            //// NOTE KI project rotation to XZ plane to keep nodes UP
            //// => nodes still travel backwards, but not rotating grazily
            //if (false) {
            //    // https://discourse.nphysics.org/t/projecting-a-unitquaternion-on-a-2d-plane/70/4
            //    const auto rotated = glm::mat3(rot) * state.m_front;
            //    //const auto front = glm::normalize(glm::vec3(rotated.x, 0, rotated.z));
            //    const auto rads = glm::atan(rotated.x, rotated.z);
            //    const auto degrees = glm::degrees(rads);
            //    rot = util::radiansToQuat(glm::vec3(0, rads, 0));

            //    dQuaternion quat{ rot.w, rot.x, rot.y, rot.z };
            //    dBodySetQuaternion(m_body.physicId, quat);
            //}
            ////const auto rotatedFront = rotBase * state.m_front;

            //if (m_geom.type == GeomType::box) {
            //    auto degrees = util::quatToDegrees(rq);
            //    KI_INFO_OUT(fmt::format(
            //        "OBJ_BODY_2: id={}, type={}, pos={}, rot={}, degrees={}",
            //        m_id, util::as_integer(m_geom.type), pos, rot, degrees));
            //}
        }

        state.setPosition(pos);
        state.setRotation(rot * state.getInvBaseRotation());
        //m_node->updateModelMatrix();
    }
}
