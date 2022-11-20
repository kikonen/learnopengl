#pragma once

#include <array>

#include <glm/glm.hpp>

//struct PlaneNew {
//    float a, b, c, d;
//
//    float distanceToPoint(const glm::vec3& pt) const
//    {
//        return a * pt.x + b * pt.y + c * pt.z + d;
//    }
//
//    void normalize()
//    {
//        float mag = std::sqrt(a * a + b * b + c * c);
//        float im = 1.0f / mag;
//        a *= im;
//        b *= im;
//        c *= im;
//        d *= im;
//    }
//};

using PlaneNew = glm::vec4;

// Fast Extraction of Viewing Frustum Planes from the World- View-Projection Matrix
// http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
// https://www.reddit.com/r/gamedev/comments/5zatbm/frustum_culling_in_opengl_glew_c/
struct FrustumNew {
    FrustumNew() noexcept = default;

    // 0 == left
    // 1 == right
    // 2 == top
    // 3 == bottom
    // 4 == near
    // 5 == far
    std::array<PlaneNew, 6> m_planes;
    //PlaneNew left;
    //PlaneNew right;
    //PlaneNew top;
    //PlaneNew bottom;
    //PlaneNew near;
    //PlaneNew far;

    void normalize();

    void normalizePlane(PlaneNew& plane);
};
