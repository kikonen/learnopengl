#include "ControllerDefinition.h"

#include <model/NodeType.h>

#include "controller/PawnController.h"
#include "controller/CameraZoomController.h"
#include "controller/SunController.h"
#include "controller/ClockController.h"

util::Ref<NodeController> ControllerDefinition::createController(
    ControllerDefinition& definition)
{
    switch (definition.m_type) {
    case ControllerType::pawn: {
        return util::Ref<PawnController>::create();
    }
    case ControllerType::camera_zoom: {
        return util::Ref<CameraZoomController>::create();
    }
    case ControllerType::sun: {
        return util::Ref<SunController>::create(definition.m_distance, false);
    }
    case ControllerType::moon: {
        return util::Ref<SunController>::create(definition.m_distance, true);
    }
    case ControllerType::clock: {
        return util::Ref<ClockController>::create(definition.m_format);
    }
    }

    return nullptr;
}
