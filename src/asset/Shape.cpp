#include "Shape.h"

#include <string>

#include "fmt/format.h"

#include "util/Util.h"

#include "asset/Shader.h"


namespace {
}

Shape::Shape()
{
}

Shape::~Shape()
{
    KI_INFO(fmt::format(
        "Shape: delete - index={}",
        m_registeredIndex));
}

const ShapeSSBO Shape::toSSBO() const
{
    return {
        m_rotation,
    };
}
