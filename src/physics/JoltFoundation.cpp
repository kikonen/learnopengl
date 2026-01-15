#include "JoltFoundation.h"

#include <thread>
#include <cstdarg>
#include <cassert>

#include <fmt/format.h>

#include "util/Log.h"

// Jolt implementation includes
JPH_SUPPRESS_WARNINGS

namespace {
    physics::JoltFoundation* s_foundation{ nullptr };

    // Configuration constants
    constexpr uint32_t MAX_BODIES = 65536;
    constexpr uint32_t NUM_BODY_MUTEXES = 0;  // 0 = default
    constexpr uint32_t MAX_BODY_PAIRS = 65536;
    constexpr uint32_t MAX_CONTACT_CONSTRAINTS = 10240;

    // Temp allocator size (10 MB)
    constexpr uint32_t TEMP_ALLOCATOR_SIZE = 10 * 1024 * 1024;

    // Jolt trace callback
    static void JoltTraceImpl(const char* inFMT, ...) {
        va_list list;
        va_start(list, inFMT);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);
        KI_INFO(fmt::format("JOLT: {}", buffer));
    }

#ifdef JPH_ENABLE_ASSERTS
    // Jolt assert callback
    static bool JoltAssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint32_t inLine) {
        KI_CRITICAL(fmt::format("JOLT ASSERT: {} : {} ({}:{})", inExpression, inMessage ? inMessage : "", inFile, inLine));
        return true; // Break into debugger
    }
#endif
}

namespace physics {

// BPLayerInterfaceImpl
BPLayerInterfaceImpl::BPLayerInterfaceImpl() {
    m_objectToBroadPhase[ObjectLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
    m_objectToBroadPhase[ObjectLayers::MOVING] = BroadPhaseLayers::MOVING;
    m_objectToBroadPhase[ObjectLayers::SENSOR] = BroadPhaseLayers::MOVING;
}

// ObjectLayerPairFilterImpl
bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const {
    switch (inObject1) {
    case ObjectLayers::NON_MOVING:
        return inObject2 == ObjectLayers::MOVING; // Non-moving only collides with moving
    case ObjectLayers::MOVING:
        return true; // Moving collides with everything
    case ObjectLayers::SENSOR:
        return false; // Sensors don't collide (used for raycasting only)
    default:
        return false;
    }
}

// ObjectVsBroadPhaseLayerFilterImpl
bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const {
    switch (inLayer1) {
    case ObjectLayers::NON_MOVING:
        return inLayer2 == BroadPhaseLayers::MOVING;
    case ObjectLayers::MOVING:
        return true;
    case ObjectLayers::SENSOR:
        return true; // Sensors can query any broadphase layer
    default:
        return false;
    }
}

// ContactListenerImpl
JPH::ValidateResult ContactListenerImpl::OnContactValidate(
    const JPH::Body& inBody1,
    const JPH::Body& inBody2,
    JPH::RVec3Arg inBaseOffset,
    const JPH::CollideShapeResult& inCollisionResult)
{
    // Accept all contacts by default
    // Custom filtering is done via object layers and collision masks stored in user data
    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
}

void ContactListenerImpl::OnContactAdded(
    const JPH::Body& inBody1,
    const JPH::Body& inBody2,
    const JPH::ContactManifold& inManifold,
    JPH::ContactSettings& ioSettings)
{
    // Apply material properties from user data if needed
    // For now, use defaults - friction and restitution are set per-body
}

void ContactListenerImpl::OnContactPersisted(
    const JPH::Body& inBody1,
    const JPH::Body& inBody2,
    const JPH::ContactManifold& inManifold,
    JPH::ContactSettings& ioSettings)
{
    // Same as OnContactAdded for persistent contacts
}

void ContactListenerImpl::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) {
    // Contact removed - nothing to do
}

// BodyActivationListenerImpl
void BodyActivationListenerImpl::OnBodyActivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData) {
    // Body activated
}

void BodyActivationListenerImpl::OnBodyDeactivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData) {
    // Body deactivated
}

// JoltFoundation static methods
void JoltFoundation::init() {
    assert(!s_foundation);
    s_foundation = new JoltFoundation();
}

void JoltFoundation::release() {
    auto* f = s_foundation;
    s_foundation = nullptr;
    delete f;
}

JoltFoundation& JoltFoundation::get() {
    assert(s_foundation);
    return *s_foundation;
}

// JoltFoundation instance methods
JoltFoundation::JoltFoundation() {
}

JoltFoundation::~JoltFoundation() {
    clear();

    if (m_initialized) {
        // Unregister types
        JPH::UnregisterTypes();

        // Destroy factory
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }
}

void JoltFoundation::prepare() {
    if (m_initialized) return;
    m_initialized = true;

    // Register allocation hook
    JPH::RegisterDefaultAllocator();

    // Install trace and assert callbacks
    JPH::Trace = JoltTraceImpl;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = JoltAssertFailedImpl;)

    // Create factory
    JPH::Factory::sInstance = new JPH::Factory();

    // Register all Jolt physics types
    JPH::RegisterTypes();

    // Create temp allocator
    m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(TEMP_ALLOCATOR_SIZE);

    // Create job system
    // Use hardware thread count minus 1 for physics (leave one for main thread)
    int numThreads = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 1);
    m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
        JPH::cMaxPhysicsJobs,
        JPH::cMaxPhysicsBarriers,
        numThreads);

    // Create physics system
    m_physicsSystem = std::make_unique<JPH::PhysicsSystem>();
    m_physicsSystem->Init(
        MAX_BODIES,
        NUM_BODY_MUTEXES,
        MAX_BODY_PAIRS,
        MAX_CONTACT_CONSTRAINTS,
        m_bpLayerInterface,
        m_objectVsBroadPhaseFilter,
        m_objectLayerPairFilter);

    // Set listeners
    m_physicsSystem->SetContactListener(&m_contactListener);
    m_physicsSystem->SetBodyActivationListener(&m_bodyActivationListener);

    KI_INFO_OUT(fmt::format("JOLT: Initialized with {} physics threads", numThreads));
}

void JoltFoundation::clear() {
    if (m_physicsSystem) {
        // Remove all bodies
        JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();

        // Get all bodies and remove them
        JPH::BodyIDVector bodies;
        m_physicsSystem->GetBodies(bodies);

        for (const JPH::BodyID& bodyId : bodies) {
            bodyInterface.RemoveBody(bodyId);
            bodyInterface.DestroyBody(bodyId);
        }
    }
}

JPH::BodyInterface& JoltFoundation::getBodyInterface() {
    return m_physicsSystem->GetBodyInterface();
}

const JPH::BodyInterface& JoltFoundation::getBodyInterface() const {
    return m_physicsSystem->GetBodyInterface();
}

} // namespace physics
