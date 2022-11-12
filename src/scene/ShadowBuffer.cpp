#include "ShadowBuffer.h"

#include <iostream>

#include "glm/glm.hpp"
#include <glm/ext.hpp>


ShadowBuffer::ShadowBuffer(const FrameBufferSpecification& spec)
    : FrameBuffer(spec)
{
}
