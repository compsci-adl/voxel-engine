#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <raylib.h>
#include <raymath.h>

struct Frustum {

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

    Vector3 planes[6]; // normal vectors representing planes
    Vector3 nearTopLeft, nearTopRight, nearBottomLeft, nearBottomRight;
    Vector3 farTopLeft, farTopRight, farBottomLeft, farBottomRight;
    float nearDistance, farDistance;
    float nearWidth, nearHeight;
    float farWidth, farHeight;
    float ratio, angle, tang;

    int PointInFrustum(const Vector3 &point);
    int SphereInFrustum(const Vector3 &point, float radius);
    int CubeInFrustum(const Vector3 &center, float x, float y, float z) const;
    void SetFrustum(const Camera3D* cam, float nearDist, float farDist);
    // void SetCamera(const Vector3 &pos, const Vector3 &target,
    //                 const Vector3 &up);
};

int Frustum::CubeInFrustum(const Vector3 &center, float x, float y, float z) const {
    int result = FRUSTUM_INSIDE;
    Vector3 halfSize = { x / 2.0f, y / 2.0f, z / 2.0f };

    // Iterate over each plane
    for (int i = 0; i < 6; i++) {
        Vector3 planeNormal = planes[i];
        Vector3 absPlaneNormal = { fabs(planeNormal.x), fabs(planeNormal.y), fabs(planeNormal.z) };

        float distance = Vector3DotProduct(planeNormal, center);
        float radius = Vector3DotProduct(absPlaneNormal, halfSize);

        if (distance + radius < 0) {
            return FRUSTUM_OUTSIDE;
        } else if (distance - radius < 0) {
            result = FRUSTUM_INTERSECT;
        }
    }

    return result;
}

// Function to create a frustum from a Camera3D
void Frustum::SetFrustum(const Camera3D* cam, float nearDist, float farDist) {

    nearDistance = nearDist;
    farDistance = farDist;

    // Calculate the forward, right, and up vectors
    Vector3 forward = Vector3Normalize(Vector3Subtract(cam->target, cam->position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, cam->up));
    Vector3 up = Vector3CrossProduct(right, forward);

    // Calculate half dimensions of near and far planes
    ratio = (float)GetScreenWidth() / (float)GetScreenHeight();
    angle = cam->fovy * DEG2RAD;
    tang = tanf(angle * 0.5f);
    nearHeight = nearDistance * tang;
    nearWidth = nearHeight * ratio;
    farHeight = farDistance * tang;
    farWidth = farHeight * ratio;

    // Calculate the center points of the near and far planes
    Vector3 nearCenter = Vector3Add(cam->position, Vector3Scale(forward, nearDistance));
    Vector3 farCenter = Vector3Add(cam->position, Vector3Scale(forward, farDistance));

    // Calculate the 4 corners of the near plane
    nearTopLeft = Vector3Add(nearCenter, Vector3Subtract(Vector3Scale(up, nearHeight), Vector3Scale(right, nearWidth)));
    nearTopRight = Vector3Add(nearCenter, Vector3Add(Vector3Scale(up, nearHeight), Vector3Scale(right, nearWidth)));
    nearBottomLeft = Vector3Subtract(nearCenter, Vector3Add(Vector3Scale(up, nearHeight), Vector3Scale(right, nearWidth)));
    nearBottomRight = Vector3Subtract(nearCenter, Vector3Subtract(Vector3Scale(up, nearHeight), Vector3Scale(right, nearWidth)));

    // Calculate the 4 corners of the far plane
    farTopLeft = Vector3Add(farCenter, Vector3Subtract(Vector3Scale(up, farHeight), Vector3Scale(right, farWidth)));
    farTopRight = Vector3Add(farCenter, Vector3Add(Vector3Scale(up, farHeight), Vector3Scale(right, farWidth)));
    farBottomLeft = Vector3Subtract(farCenter, Vector3Add(Vector3Scale(up, farHeight), Vector3Scale(right, farWidth)));
    farBottomRight = Vector3Subtract(farCenter, Vector3Subtract(Vector3Scale(up, farHeight), Vector3Scale(right, farWidth)));

    // Planes
    planes[Frustum::FRUSTUM_TOP] = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(nearTopRight, cam->position), Vector3Subtract(nearTopLeft, cam->position)));
    planes[Frustum::FRUSTUM_BOTTOM] = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(nearBottomLeft, cam->position), Vector3Subtract(nearBottomRight, cam->position)));
    planes[Frustum::FRUSTUM_LEFT] = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(nearTopLeft, cam->position), Vector3Subtract(nearBottomLeft, cam->position)));
    planes[Frustum::FRUSTUM_RIGHT] = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(nearBottomRight, cam->position), Vector3Subtract(nearTopRight, cam->position)));

    planes[Frustum::FRUSTUM_NEAR] = forward;
    planes[Frustum::FRUSTUM_FAR] = Vector3Negate(forward);

}

#endif