#include "Path.h"

namespace nav
{
    void Path::setWaypoints(float* waypoints, int count)
    {
        m_waypoints.clear();
        m_waypoints.reserve(count);
        for (int i = 0; i < count; i++)
        {
            m_waypoints.emplace_back(
                waypoints[i * 3 + 0],
                waypoints[i * 3 + 1],
                waypoints[i * 3 + 2]);
        }
    }
}
