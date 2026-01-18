#include "Object.h"

#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyInterface.h>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/Log.h"
#include "util/util.h"
#include "util/glm_util.h"

#include "model/Node.h"

#include "registry/NodeRegistry.h"

#include "jolt_util.h"

namespace {
    inline glm::vec3 UP{ 0.f, 1.f, 0.f };

    const glm::quat IDENTITY_QUAT{ 0.f, 0.f, 0.f, 0.f };
}

namespace physics
{
    Object::Object() = default;
    Object::~Object() = default;

    void Object::swap(Object& o) noexcept
    {
        std::swap(m_body, o.m_body);
        std::swap(m_shape, o.m_shape);
    }

    void Object::release(JPH::BodyInterface& bodyInterface)
    {
        m_body.release(bodyInterface);
        m_shape.release(bodyInterface);
    }

    void Object::create(
        physics::object_id objectId,
        uint32_t entityIndex,
        JPH::PhysicsSystem& physicsSystem,
        NodeRegistry& nodeRegistry)
    {
        if (ready()) return;

        //glm::vec3 scale{ 1.f };

        // NOTE KI assumping that parent scale does not affect node
        // => 99,9% true for physics nodes, i.e. does not make sense
        // for them to have parent node affecting scale
        const auto& state = nodeRegistry.getState(entityIndex);
        m_scale = state.getScale();

        // Create body first (if exists)
        m_body.create(
            objectId,
            m_shape.category,
            m_shape.collisionMask,
            physicsSystem,
            m_scale);

        // Create shape (for standalone shapes, creates static body)
        m_shape.create(
            objectId,
            physicsSystem,
            m_scale,
            m_body.m_bodyId);
    }

    bool Object::updateToPhysics(
        uint32_t entityIndex,
        ki::level_id& matrixLevel,
        JPH::BodyInterface& bodyInterface,
        NodeRegistry& nodeRegistry)
    {
        const auto& state = nodeRegistry.getState(entityIndex);
        {
            const auto level = state.getMatrixLevel();
            if (matrixLevel == level) return false;
            matrixLevel = level;
        }

        const glm::vec3& pos = state.getWorldPosition();
        const glm::vec3& pivot = state.getWorldPivot();
        const auto& rot = state.getModelRotation() * m_body.baseRotation;

        if (m_body.hasPhysicsBody()) {
            m_body.updatePhysic(bodyInterface, pivot, pos, rot);

            // Reset angular velocity and torque for kinematic bodies
            m_body.setAngularVelocity(bodyInterface, glm::vec3(0.f));
        }
        else if (m_shape.hasPhysicsBody())
        {
            // NOTE KI for "shape only" nodes
            m_shape.updatePhysics(bodyInterface, pivot, pos, rot);
        }

        return true;
    }

    void Object::updateFromPhysics(
        uint32_t entityIndex,
        const JPH::BodyInterface& bodyInterface,
        NodeRegistry& nodeRegistry) const
    {
        // NOTE KI "shape only" is not updated back to node
        if (!m_body.hasPhysicsBody()) return;
        if (m_body.kinematic) return;

        auto& state = nodeRegistry.modifyState(entityIndex);
        auto& parentState = nodeRegistry.getParentState(entityIndex);

        glm::vec3 pos{ 0.f };
        glm::quat rot{ 1.f, 0.f, 0.f, 0.f };
        {
            pos = m_body.getPhysicPosition(bodyInterface);
            rot = m_body.getPhysicRotation(bodyInterface);

            // NOTE KI parent *SHOULD* be root (== null) for all physics nodes
            // => otherwise math does not make sense
            // => for NodeGenerator based nodes parent is generator container
            pos -= parentState.getWorldPosition();
            pos -= (state.getWorldPivot() - state.getWorldPosition());

            // https://danceswithcode.net/engineeringnotes/quaternions/quaternions.html
            rot = glm::normalize(rot * m_body.invBaseRotation);
        }

        state.setPosition(pos);
        state.setRotation(rot * state.getInvBaseRotation());
    }
}
