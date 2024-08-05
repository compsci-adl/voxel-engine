#include "raylib.h"
#include "rcamera.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <math.h>
#include "ChunkManager.h"

const int referenceScreenWidth = 800;
const int referenceScreenHeight = 640;

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
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);    // Window configuration flags
    InitWindow(screenWidth, screenHeight, "voxel-engine");

    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = { 0.0f, 2.0f, 4.0f };    // Camera position
    camera.target = { 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    int cameraMode = CAMERA_FREE;

    DisableCursor();                    // Limit cursor to relative movement inside the window
    GuiSetStyle(LABEL, TEXT + (guiState * 3), 0xff00ff);

    // SetTargetFPS(144);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    bool fullscreen = false;

    // Chunk chunk = Chunk({0.0, 0.0, 0.0});
    // chunk.randomize();
    // chunk.load();
    // chunk.setup();
    ChunkManager chunkManager = ChunkManager(3);

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Switch camera mode
        if (IsKeyPressed(KEY_ONE))
        {
            cameraMode = CAMERA_FREE;
            camera.up = { 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_TWO))
        {
            cameraMode = CAMERA_FIRST_PERSON;
            camera.up = { 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_THREE))
        {
            cameraMode = CAMERA_THIRD_PERSON;
            camera.up = { 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_FOUR))
        {
            cameraMode = CAMERA_ORBITAL;
            camera.up = { 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        // Switch camera projection
        if (IsKeyPressed(KEY_P))
        {
            if (camera.projection == CAMERA_PERSPECTIVE)
            {
                // Create isometric view
                cameraMode = CAMERA_THIRD_PERSON;
                // Note: The target distance is related to the render distance in the orthographic projection
                camera.position = { 0.0f, 2.0f, -100.0f };
                camera.target = { 0.0f, 0.0f, 0.0f };
                camera.up = { 0.0f, 1.0f, 0.0f };
                camera.projection = CAMERA_ORTHOGRAPHIC;
                camera.fovy = 20.0f; // near plane width in CAMERA_ORTHOGRAPHIC
                CameraYaw(&camera, -135 * DEG2RAD, true);
                CameraPitch(&camera, -45 * DEG2RAD, true, true, false);
            }
            else if (camera.projection == CAMERA_ORTHOGRAPHIC)
            {
                // Reset to default view
                cameraMode = CAMERA_THIRD_PERSON;
                camera.position = { 0.0f, 2.0f, 10.0f };
                camera.target = { 0.0f, 2.0f, 0.0f };
                camera.up = { 0.0f, 1.0f, 0.0f };
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

        if (IsKeyPressed(KEY_X)) {
            chunkManager.genChunk = !chunkManager.genChunk;
        }

        // Update camera computes movement internally depending on the camera mode
        // Some default standard keyboard/mouse inputs are hardcoded to simplify use
        // For advanced camera controls, it's recommended to compute camera movement manually
        UpdateCamera(&camera, cameraMode);                  // Update camera

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKGRAY);

            BeginMode3D(camera);

                // Draw cubes
                // DrawCube({ -1.6f, -1.6f, -1.6f }, 1.6f, 1.6f, 1.6f, BLUE);     // Draw a blue wall
                DrawCubeWires({ 0.0f, 0.0f, 0.0f }, 10.0f, 10.0f, 10.0f, BLACK);     // Draw a blue wall
                // chunk.render();
                chunkManager.Update(0.0, camera.position, camera.target);
                chunkManager.Render();

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
            GuiLabel({ 15 * scale, 15, 300 * scale, 10 * scale }, "Camera controls:");
            GuiLabel({ 15 * scale, 30, 300 * scale, 10 * scale }, "- Move keys: W, A, S, D, Space, Left-Ctrl");
            GuiLabel({ 15 * scale, 45, 300 * scale, 10 * scale }, "- Look around: arrow keys or mouse");
            GuiLabel({ 15 * scale, 60, 300 * scale, 10 * scale }, "- Camera mode keys: 1, 2, 3, 4");
            GuiLabel({ 15 * scale, 75, 300 * scale, 10 * scale }, "- Zoom keys: num-plus, num-minus or mouse scroll");
            GuiLabel({ 15 * scale, 90, 300 * scale, 10 * scale }, "- Camera projection key: P");
            GuiLabel({ 15 * scale, 105, 300 * scale, 10 * scale}, "- Toggle Chunk Gen: X");
            GuiLabel({ 15 * scale, 120, 300 * scale, 10 * scale}, "- Toggle fullscreen: F11");
            GuiLabel({ 15 * scale, 135, 300 * scale, 10 * scale}, TextFormat("- GetFPS: %i", GetFPS()));
            GuiLabel({ 15 * scale, 150, 300 * scale, 10 * scale}, TextFormat("- Chunk Gen: %s", chunkManager.genChunk ? "On" : "Off"));

            GuiLabel({ 976 * scale, 15, 300 * scale, 10 * scale }, "Camera status:");
            GuiLabel({ 976 * scale, 30, 300 * scale, 10 * scale }, TextFormat("- Mode: %s", (cameraMode == CAMERA_FREE) ? "FREE" :
                                                                                                          (cameraMode == CAMERA_FIRST_PERSON) ? "FIRST_PERSON" :
                                                                                                          (cameraMode == CAMERA_THIRD_PERSON) ? "THIRD_PERSON" :
                                                                                                          (cameraMode == CAMERA_ORBITAL) ? "ORBITAL" : "CUSTOM"));
            GuiLabel({ 976 * scale, 45, 300 * scale, 10 * scale }, TextFormat("- Projection: %s", (camera.projection == CAMERA_PERSPECTIVE) ? "PERSPECTIVE" :
                                                                                                                (camera.projection == CAMERA_ORTHOGRAPHIC) ? "ORTHOGRAPHIC" : "CUSTOM"));
            GuiLabel({ 976 * scale, 60, 300 * scale, 10 * scale }, TextFormat("- Position: (%06.3f, %06.3f, %06.3f)", camera.position.x, camera.position.y, camera.position.z));
            GuiLabel({ 976 * scale, 75, 300 * scale, 10 * scale }, TextFormat("- Target: (%06.3f, %06.3f, %06.3f)", camera.target.x, camera.target.y, camera.target.z));
            GuiLabel({ 976 * scale, 90, 300 * scale, 10 * scale }, TextFormat("- Up: (%06.3f, %06.3f, %06.3f)", camera.up.x, camera.up.y, camera.up.z));

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

