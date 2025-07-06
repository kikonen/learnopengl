#include "ControllerDefinition.h"

#include <model/NodeType.h>

#include "controller/PawnController.h"
#include "controller/CameraZoomController.h"

std::unique_ptr<NodeController> ControllerDefinition::createController(
    ControllerDefinition& definition)
{
    switch (definition.m_type) {
    case ControllerType::pawn: {
        return std::make_unique<PawnController>();
    }
    case ControllerType::camera_zoom: {
        return std::make_unique<CameraZoomController>();
    }
    }

    return nullptr;
}
