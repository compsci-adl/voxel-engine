#ifndef CAMERA_H
#define CAMERA_H

#include "Frustum.h"

// Frustum createFrustumFromCamera(Camera camera);

struct Camera {
    // TODO: figure out camera following
    // Entity* attachedTo;
    bool isFree = true;
    // camera
    float cameraSpeedMultiplier = 20.0f;
    glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 1.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);

    // Calculate the left vector (opposite of right)
    glm::vec3 cameraLeft = -cameraRight;

    // The top vector is the same as the up vector in this case
    glm::vec3 cameraTop = cameraUp;

    bool firstMouse = true;
    float yaw = -90.0f; // yaw is initialized to -90.0 degrees since a yaw of
                        // 0.0 results in a direction vector pointing to the
                        // right so we initially rotate a bit to the left.
    float pitch = 0.0f;
    float lastX = 800.0f / 2.0;
    float lastY = 600.0 / 2.0;
    float fov = 45.0f;

    float zNear = 0.1f;
    float zFar = 1000.0f;

    Frustum frustum;

    Camera();
};

Frustum createFrustumFromCamera(Camera camera) {
    Frustum frustum;

    float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    const float halfVSide =
        camera.zFar * tanf((float)glm::radians(camera.fov) * .5f);
    const float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = camera.zFar * camera.cameraFront;

    frustum.planes[Frustum::FRUSTUM_NEAR] =
        Plane3(camera.cameraPos + camera.zNear * camera.cameraFront,
               camera.cameraFront);
    camera.frustum.planes[Frustum::FRUSTUM_FAR] =
        Plane3(camera.cameraPos + frontMultFar, -camera.cameraFront);
    camera.frustum.planes[Frustum::FRUSTUM_RIGHT] =
        Plane3(camera.cameraPos,
               glm::cross(frontMultFar - camera.cameraRight * halfHSide,
                          camera.cameraUp));
    frustum.planes[Frustum::FRUSTUM_LEFT] =
        Plane3(camera.cameraPos,
               glm::cross(camera.cameraUp,
                          frontMultFar + camera.cameraRight * halfHSide));
    frustum.planes[Frustum::FRUSTUM_TOP] =
        Plane3(camera.cameraPos,
               glm::cross(camera.cameraRight,
                          frontMultFar - camera.cameraUp * halfVSide));
    frustum.planes[Frustum::FRUSTUM_BOTTOM] = Plane3(
        camera.cameraPos, glm::cross(frontMultFar + camera.cameraUp * halfVSide,
                                     camera.cameraRight));

    return frustum;
}

Camera::Camera() { frustum = createFrustumFromCamera(*this); }

#endif // CAMERA_H