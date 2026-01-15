#pragma once

#include <memory>
#include <mutex>

// Jolt configuration - must match vcpkg build settings
#define JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
#define JPH_OBJECT_STREAM

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/ContactListener.h>

#include "Category.h"

namespace physics {

// Jolt broadphase layers
namespace BroadPhaseLayers {
    static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
    static constexpr uint32_t NUM_LAYERS = 2;
}

// Jolt object layers - maps to collision categories
namespace ObjectLayers {
    static constexpr JPH::ObjectLayer NON_MOVING = 0;  // Static geometry (terrain, scenery)
    static constexpr JPH::ObjectLayer MOVING = 1;      // Dynamic bodies
    static constexpr JPH::ObjectLayer SENSOR = 2;      // Sensors/triggers (no collision response)
    static constexpr uint32_t NUM_LAYERS = 3;
}

// Maps object layer to broadphase layer
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
    BPLayerInterfaceImpl();

    virtual uint32_t GetNumBroadPhaseLayers() const override {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
        return m_objectToBroadPhase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
        switch ((JPH::BroadPhaseLayer::Type)inLayer) {
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING: return "NON_MOVING";
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING: return "MOVING";
        default: return "UNKNOWN";
        }
    }
#endif

private:
    JPH::BroadPhaseLayer m_objectToBroadPhase[ObjectLayers::NUM_LAYERS];
};

// Determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override;
};

// Determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override;
};

// Handles contact events
class ContactListenerImpl : public JPH::ContactListener {
public:
    virtual JPH::ValidateResult OnContactValidate(
        const JPH::Body& inBody1,
        const JPH::Body& inBody2,
        JPH::RVec3Arg inBaseOffset,
        const JPH::CollideShapeResult& inCollisionResult) override;

    virtual void OnContactAdded(
        const JPH::Body& inBody1,
        const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold,
        JPH::ContactSettings& ioSettings) override;

    virtual void OnContactPersisted(
        const JPH::Body& inBody1,
        const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold,
        JPH::ContactSettings& ioSettings) override;

    virtual void OnContactRemoved(
        const JPH::SubShapeIDPair& inSubShapePair) override;
};

// Body activation listener for debugging
class BodyActivationListenerImpl : public JPH::BodyActivationListener {
public:
    virtual void OnBodyActivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData) override;
    virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData) override;
};

// Jolt physics foundation - manages Jolt system lifecycle
class JoltFoundation {
public:
    static void init();
    static void release();
    static JoltFoundation& get();

    JoltFoundation();
    ~JoltFoundation();

    JoltFoundation(const JoltFoundation&) = delete;
    JoltFoundation& operator=(const JoltFoundation&) = delete;

    void prepare();
    void clear();

    JPH::PhysicsSystem& getPhysicsSystem() { return *m_physicsSystem; }
    const JPH::PhysicsSystem& getPhysicsSystem() const { return *m_physicsSystem; }

    JPH::BodyInterface& getBodyInterface();
    const JPH::BodyInterface& getBodyInterface() const;

    JPH::TempAllocator* getTempAllocator() { return m_tempAllocator.get(); }
    JPH::JobSystem* getJobSystem() { return m_jobSystem.get(); }

    // Get filters for queries
    const BPLayerInterfaceImpl& getBPLayerInterface() const { return m_bpLayerInterface; }
    const ObjectLayerPairFilterImpl& getObjectLayerPairFilter() const { return m_objectLayerPairFilter; }
    const ObjectVsBroadPhaseLayerFilterImpl& getObjectVsBroadPhaseFilter() const { return m_objectVsBroadPhaseFilter; }

private:
    bool m_initialized{ false };

    // Jolt foundation objects
    std::unique_ptr<JPH::TempAllocator> m_tempAllocator;
    std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
    std::unique_ptr<JPH::PhysicsSystem> m_physicsSystem;

    // Layer interfaces
    BPLayerInterfaceImpl m_bpLayerInterface;
    ObjectLayerPairFilterImpl m_objectLayerPairFilter;
    ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadPhaseFilter;

    // Listeners
    ContactListenerImpl m_contactListener;
    BodyActivationListenerImpl m_bodyActivationListener;
};

} // namespace physics
