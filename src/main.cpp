#include "raylib.h"
#include "rcamera.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <math.h>

#define MAX_COLUMNS 20

const int referenceScreenWidth = 800;
const int referenceScreenHeight = 450;

// Function to calculate the scaling factor
float GetScalingFactor(int currentWidth, int currentHeight) {
    float scaleX = (float)currentWidth / referenceScreenWidth;
    float scaleY = (float)currentHeight / referenceScreenHeight;
    return fminf(scaleX, scaleY);
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 600;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);    // Window configuration flags
    InitWindow(screenWidth, screenHeight, "voxel-engine");

    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 4.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    int cameraMode = CAMERA_FREE;

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    bool fullscreen = false;

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Switch camera mode
        if (IsKeyPressed(KEY_ONE))
        {
            cameraMode = CAMERA_FREE;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_TWO))
        {
            cameraMode = CAMERA_FIRST_PERSON;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_THREE))
        {
            cameraMode = CAMERA_THIRD_PERSON;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_FOUR))
        {
            cameraMode = CAMERA_ORBITAL;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        // Switch camera projection
        if (IsKeyPressed(KEY_P))
        {
            if (camera.projection == CAMERA_PERSPECTIVE)
            {
                // Create isometric view
                cameraMode = CAMERA_THIRD_PERSON;
                // Note: The target distance is related to the render distance in the orthographic projection
                camera.position = (Vector3){ 0.0f, 2.0f, -100.0f };
                camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
                camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
                camera.projection = CAMERA_ORTHOGRAPHIC;
                camera.fovy = 20.0f; // near plane width in CAMERA_ORTHOGRAPHIC
                CameraYaw(&camera, -135 * DEG2RAD, true);
                CameraPitch(&camera, -45 * DEG2RAD, true, true, false);
            }
            else if (camera.projection == CAMERA_ORTHOGRAPHIC)
            {
                // Reset to default view
                cameraMode = CAMERA_THIRD_PERSON;
                camera.position = (Vector3){ 0.0f, 2.0f, 10.0f };
                camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
                camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
                camera.projection = CAMERA_PERSPECTIVE;
                camera.fovy = 60.0f;
            }
        }

        // Toggle fullscreen
        if (IsKeyPressed(KEY_F11))
        {
            fullscreen = !fullscreen;
            ToggleFullscreen();
        }

        // Update camera computes movement internally depending on the camera mode
        // Some default standard keyboard/mouse inputs are hardcoded to simplify use
        // For advanced camera controls, it's recommended to compute camera movement manually
        UpdateCamera(&camera, cameraMode);                  // Update camera

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawCube((Vector3){ -8.0f, -8.0f, -8.0f }, 8.0f, 8.0f, 8.0f, BLUE);     // Draw a blue wall
                DrawCubeWires((Vector3){ -8.0f, -8.0f, -8.0f }, 8.0f, 8.0f, 8.0f, BLACK);     // Draw a blue wall

                // Draw player cube
                if (cameraMode == CAMERA_THIRD_PERSON)
                {
                    DrawCube(camera.target, 0.5f, 0.5f, 0.5f, PURPLE);
                    DrawCubeWires(camera.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);
                }

            EndMode3D();

            // Get current screen width and height
            int currentScreenWidth = GetScreenWidth();
            int currentScreenHeight = GetScreenHeight();

            // Calculate scaling factor
            float scale = GetScalingFactor(currentScreenWidth, currentScreenHeight);

            // GUI controls using raygui
            GuiLabel((Rectangle){ 15 * scale, 15, 200 * scale, 10 * scale }, "Camera controls:");
            GuiLabel((Rectangle){ 15 * scale, 30, 200 * scale, 10 * scale }, "- Move keys: W, A, S, D, Space, Left-Ctrl");
            GuiLabel((Rectangle){ 15 * scale, 45, 200 * scale, 10 * scale }, "- Look around: arrow keys or mouse");
            GuiLabel((Rectangle){ 15 * scale, 60, 200 * scale, 10 * scale }, "- Camera mode keys: 1, 2, 3, 4");
            GuiLabel((Rectangle){ 15 * scale, 75, 200 * scale, 10 * scale }, "- Zoom keys: num-plus, num-minus or mouse scroll");
            GuiLabel((Rectangle){ 15 * scale, 90, 200 * scale, 10 * scale }, "- Camera projection key: P");
            GuiLabel((Rectangle){ 15 * scale, 105, 200 * scale, 10 * scale}, "- Toggle fullscreen: F11");

            GuiLabel((Rectangle){ 610 * scale, 15, 200 * scale, 10 * scale }, "Camera status:");
            GuiLabel((Rectangle){ 610 * scale, 30, 200 * scale, 10 * scale }, TextFormat("- Mode: %s", (cameraMode == CAMERA_FREE) ? "FREE" :
                                                                                                          (cameraMode == CAMERA_FIRST_PERSON) ? "FIRST_PERSON" :
                                                                                                          (cameraMode == CAMERA_THIRD_PERSON) ? "THIRD_PERSON" :
                                                                                                          (cameraMode == CAMERA_ORBITAL) ? "ORBITAL" : "CUSTOM"));
            GuiLabel((Rectangle){ 610 * scale, 45, 200 * scale, 10 * scale }, TextFormat("- Projection: %s", (camera.projection == CAMERA_PERSPECTIVE) ? "PERSPECTIVE" :
                                                                                                                (camera.projection == CAMERA_ORTHOGRAPHIC) ? "ORTHOGRAPHIC" : "CUSTOM"));
            GuiLabel((Rectangle){ 610 * scale, 60, 200 * scale, 10 * scale }, TextFormat("- Position: (%06.3f, %06.3f, %06.3f)", camera.position.x, camera.position.y, camera.position.z));
            GuiLabel((Rectangle){ 610 * scale, 75, 200 * scale, 10 * scale }, TextFormat("- Target: (%06.3f, %06.3f, %06.3f)", camera.target.x, camera.target.y, camera.target.z));
            GuiLabel((Rectangle){ 610 * scale, 90, 200 * scale, 10 * scale }, TextFormat("- Up: (%06.3f, %06.3f, %06.3f)", camera.up.x, camera.up.y, camera.up.z));

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

