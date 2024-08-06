#pragma once
#include "raylib.h"
#include <raymath.h>
#include <rcamera.h>

#ifndef CAMERA_ROTATION_SPEED
#define CAMERA_ROTATION_SPEED 0.03f
#endif

#ifndef CAMERA_PAN_SPEED
#define CAMERA_PAN_SPEED 0.2f
#endif

// Camera mouse movement sensitivity
#ifndef CAMERA_MOUSE_MOVE_SENSITIVITY
#define CAMERA_MOUSE_MOVE_SENSITIVITY                                          \
    0.003f // TODO: it should be independant of framerate
#endif

// Camera orbital speed in CAMERA_ORBITAL mode
#ifndef CAMERA_ORBITAL_SPEED
#define CAMERA_ORBITAL_SPEED 0.5f // Radians per second
#endif

void CustomUpdateCamera(Camera *camera, int mode, float movementSpeed) {
    Vector2 mousePositionDelta = GetMouseDelta();

    bool moveInWorldPlane =
        ((mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON));
    bool rotateAroundTarget =
        ((mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
    bool lockView = ((mode == CAMERA_FREE) || (mode == CAMERA_FIRST_PERSON) ||
                     (mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
    bool rotateUp = false;

    if (mode == CAMERA_CUSTOM) {
    } else if (mode == CAMERA_ORBITAL) {
        // Orbital can just orbit
        Matrix rotation = MatrixRotate(GetCameraUp(camera),
                                       CAMERA_ORBITAL_SPEED * GetFrameTime());
        Vector3 view = Vector3Subtract(camera->position, camera->target);
        view = Vector3Transform(view, rotation);
        camera->position = Vector3Add(camera->target, view);
    } else {
        // Camera rotation
        if (IsKeyDown(KEY_DOWN))
            CameraPitch(camera, -CAMERA_ROTATION_SPEED, lockView,
                        rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_UP))
            CameraPitch(camera, CAMERA_ROTATION_SPEED, lockView,
                        rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_RIGHT))
            CameraYaw(camera, -CAMERA_ROTATION_SPEED, rotateAroundTarget);
        if (IsKeyDown(KEY_LEFT))
            CameraYaw(camera, CAMERA_ROTATION_SPEED, rotateAroundTarget);
        if (IsKeyDown(KEY_Q))
            CameraRoll(camera, -CAMERA_ROTATION_SPEED);
        if (IsKeyDown(KEY_E))
            CameraRoll(camera, CAMERA_ROTATION_SPEED);

        // Camera movement
        // Camera pan (for CAMERA_FREE)
        if ((mode == CAMERA_FREE) && (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))) {
            const Vector2 mouseDelta = GetMouseDelta();
            if (mouseDelta.x > 0.0f)
                CameraMoveRight(camera, CAMERA_PAN_SPEED, moveInWorldPlane);
            if (mouseDelta.x < 0.0f)
                CameraMoveRight(camera, -CAMERA_PAN_SPEED, moveInWorldPlane);
            if (mouseDelta.y > 0.0f)
                CameraMoveUp(camera, -CAMERA_PAN_SPEED);
            if (mouseDelta.y < 0.0f)
                CameraMoveUp(camera, CAMERA_PAN_SPEED);
        } else {
            // Mouse support
            CameraYaw(camera,
                      -mousePositionDelta.x * CAMERA_MOUSE_MOVE_SENSITIVITY,
                      rotateAroundTarget);
            CameraPitch(camera,
                        -mousePositionDelta.y * CAMERA_MOUSE_MOVE_SENSITIVITY,
                        lockView, rotateAroundTarget, rotateUp);
        }

        // Keyboard support
        if (IsKeyDown(KEY_W))
            CameraMoveForward(camera, movementSpeed, moveInWorldPlane);
        if (IsKeyDown(KEY_A))
            CameraMoveRight(camera, -movementSpeed, moveInWorldPlane);
        if (IsKeyDown(KEY_S))
            CameraMoveForward(camera, -movementSpeed, moveInWorldPlane);
        if (IsKeyDown(KEY_D))
            CameraMoveRight(camera, movementSpeed, moveInWorldPlane);

        // Gamepad movement
        if (IsGamepadAvailable(0)) {
            // Gamepad controller support
            CameraYaw(camera,
                      -(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X) * 2) *
                          CAMERA_MOUSE_MOVE_SENSITIVITY,
                      rotateAroundTarget);
            CameraPitch(camera,
                        -(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y) * 2) *
                            CAMERA_MOUSE_MOVE_SENSITIVITY,
                        lockView, rotateAroundTarget, rotateUp);

            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) <= -0.25f)
                CameraMoveForward(camera, movementSpeed, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) <= -0.25f)
                CameraMoveRight(camera, -movementSpeed, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) >= 0.25f)
                CameraMoveForward(camera, -movementSpeed, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) >= 0.25f)
                CameraMoveRight(camera, movementSpeed, moveInWorldPlane);
        }

        if (mode == CAMERA_FREE) {
            if (IsKeyDown(KEY_SPACE))
                CameraMoveUp(camera, movementSpeed);
            if (IsKeyDown(KEY_LEFT_CONTROL))
                CameraMoveUp(camera, -movementSpeed);
        }
    }

    if ((mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL) ||
        (mode == CAMERA_FREE)) {
        // Zoom target distance
        CameraMoveToTarget(camera, -GetMouseWheelMove());
        if (IsKeyPressed(KEY_KP_SUBTRACT))
            CameraMoveToTarget(camera, 2.0f);
        if (IsKeyPressed(KEY_KP_ADD))
            CameraMoveToTarget(camera, -2.0f);
    }
}
