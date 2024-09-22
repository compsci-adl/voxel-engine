#ifndef FRUSTUM_H
#define FRUSTUM_H

// #include "Camera.h"
#include "smolgl.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

struct Plane3 {
    // unit vector
    glm::vec3 normal = {0.f, 1.f, 0.f};

    // distance from origin to the nearest point in the plane
    float distance;
    Plane3();
    // Plane3(glm::vec3 _normal, glm::vec3 _origin);
    Plane3(const glm::vec3 &p1, const glm::vec3 &norm);
    float GetPointDistance(glm::vec3 point);
};

Plane3::Plane3() {
    normal = {0.f, 1.f, 0.f};
    distance = 0.f;
}

// Plane3::Plane3(glm::vec3 _origin, glm::vec3 _normal) {
//     normal = _normal;
//     origin = _origin;
// }

Plane3::Plane3(const glm::vec3 &p1, const glm::vec3 &norm) {
    normal = glm::normalize(norm);
    distance = glm::dot(normal, p1);
}

float Plane3::GetPointDistance(glm::vec3 point) {
    return glm::dot(point, normal) - distance;
    // float numerator = std::abs((normal.x * point.x + normal.y * point.y +
    //                            normal.z * point.z) + distance);
    // float denominator =
    //     std::sqrtf(std::powf(normal.x, 2) + std::powf(normal.y, 2) +
    //                std::powf(normal.z, 2));
    // return numerator / denominator;
}

struct Frustum {
    Frustum();
    ~Frustum();
    void SetFrustum(float angle, float ratio, float nearD, float farD);
    void SetCamera(const glm::vec3 &pos, const glm::vec3 &target,
                   const glm::vec3 &up);
    int PointInFrustum(const glm::vec3 &point);
    int SphereInFrustum(const glm::vec3 &point, float radius);
    int CubeInFrustum(const glm::vec3 &center, float x, float y, float z);

    enum {
        FRUSTUM_TOP = 0,
        FRUSTUM_BOTTOM,
        FRUSTUM_LEFT,
        FRUSTUM_RIGHT,
        FRUSTUM_NEAR,
        FRUSTUM_FAR,
    };

    enum {
        FRUSTUM_OUTSIDE = 0,
        FRUSTUM_INTERSECT,
        FRUSTUM_INSIDE,
    };
    Plane3 planes[6];
    glm::vec3 nearTopLeft, nearTopRight, nearBottomLeft, nearBottomRight;
    glm::vec3 farTopLeft, farTopRight, farBottomLeft, farBottomRight;
    float nearDistance, farDistance;
    float nearWidth, nearHeight;
    float farWidth, farHeight;
    float ratio, angle, tang;
};

Frustum::Frustum() {}
Frustum::~Frustum() {}

int Frustum::SphereInFrustum(const glm::vec3 &point, float radius) {
    int result = FRUSTUM_INSIDE;
    float distance;
    for (int i = 0; i < 6; i++) {
        if (i == FRUSTUM_FAR || i == FRUSTUM_NEAR)
            continue;
        distance = planes[i].GetPointDistance(point);
        // distance = glm::dot(point, planes[i].normal) - planes[i].distance;
        if (distance < -radius) {
            return FRUSTUM_OUTSIDE;
        } else if (distance < radius) {
            result = FRUSTUM_INTERSECT;
        }
    }
    return (result);
}

int Frustum::CubeInFrustum(const glm::vec3 &center, float x, float y, float z) {
    // NOTE : This code can be optimized, it is just easier to read and
    // understand as is
    int result = FRUSTUM_INSIDE;
    for (int i = 0; i < 6; i++) { // Reset counters for corners in and out
        int out = 0;
        int in = 0;
        if (planes[i].GetPointDistance(center + glm::vec3(-x, -y, -z)) < 0) {
            out++;
        } else {
            in++;
        }
        if (planes[i].GetPointDistance(center + glm::vec3(x, -y, -z)) < 0) {
            out++;
        } else {
            in++;
        }
        if (planes[i].GetPointDistance(center + glm::vec3(-x, -y, z)) < 0) {
            out++;
        } else {
            in++;
        }
        if (planes[i].GetPointDistance(center + glm::vec3(x, -y, z)) < 0) {
            out++;
        } else {
            in++;
        }
        if (planes[i].GetPointDistance(center + glm::vec3(-x, y, -z)) < 0) {
            out++;
        } else {
            in++;
        }
        if (planes[i].GetPointDistance(center + glm::vec3(x, y, -z)) < 0) {
            out++;
        } else {
            in++;
        }
        if (planes[i].GetPointDistance(center + glm::vec3(-x, y, z)) < 0) {
            out++;
        } else {
            in++;
        }
        if (planes[i].GetPointDistance(center + glm::vec3(x, y, z)) < 0) {
            out++;
        } else {
            in++;
        } // If all corners are out
        if (!in) {
            return FRUSTUM_OUTSIDE;
        } // If some corners are out and others are in
        else if (out) {
            result = FRUSTUM_INTERSECT;
        }
    }
    return (result);
}

#endif // FRUSTUM_H
