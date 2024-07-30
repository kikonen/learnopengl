#include "Spline.h"

glm::vec3 Spline::calculatePosition(size_t startIndex, float t)
{
    // Check if startIdx is out of bounds
    if (startIndex >= m_controlPoints.size())
    {
        return m_controlPoints.back();
    }
    else if (startIndex == 0)
    {
        return m_controlPoints[startIndex];
    }
    else if (startIndex + 2 >= m_controlPoints.size())
    {
        return m_controlPoints[startIndex];
    }

    // Get p0 through p3
    const auto& p0 = m_controlPoints[startIndex - 1];
    const auto& p1 = m_controlPoints[startIndex];
    const auto& p2 = m_controlPoints[startIndex + 1];
    const auto& p3 = m_controlPoints[startIndex + 2];

    // Compute position according to Catmull-Rom equation
    //
    // Catmull-Rom
    // p(t) = 0.5 * (2 * p1 + (-p0 0 p2) * t + (2 * p0 - 5 * p1 + 4 * p2 - p3) * t^2 + (-p0 + 3 * p1 - 3 * p2 + p3) * t^3)
    //
    const auto position =
        0.5f * ((2.0f * p1) + (-1.0f * p0 + p2) * t +
        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
        (-1.0f * p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t);

    return position;
}

