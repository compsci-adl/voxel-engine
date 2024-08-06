#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "ChunkManager.h"
#include "raygui.h"
#include "utils.h"

#include <limits>

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
int main(void) {
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE); // Window configuration flags
    InitWindow(screenWidth, screenHeight, "voxel-engine");

    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = {0};
    camera.position = {0.0f, 2.0f, 0.01f}; // Camera position
    camera.target = {0.0f, 2.0f, 0.0f};    // Camera looking at point
    camera.up = {0.0f, 1.0f,
                 0.0f};  // Camera up vector (rotation towards target)
    camera.fovy = 60.0f; // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE; // Camera projection type

    // settings menu
    bool showSettings = false;
    bool editMode = false;

    // settings
    float movementSpeed = 0.1f;

    int cameraMode = CAMERA_FREE;

    DisableCursor(); // Limit cursor to relative movement inside the window
    GuiSetStyle(LABEL, TEXT + (guiState * 3), 0xff00ff);
    GuiSetStyle(SLIDER, TEXT + (guiState * 3), 0xff00ff);

    // SetTargetFPS(144);                   // Set our game to run at 60
    // frames-per-second
    //--------------------------------------------------------------------------------------

    bool fullscreen = false;

    // Chunk chunk = Chunk({0.0, 0.0, 0.0});
    // chunk.randomize();
    // chunk.load();
    // chunk.setup();
    ChunkManager chunkManager = ChunkManager(3);
    Ray mouseRay = {0};

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Switch camera mode
        if (IsKeyPressed(KEY_ONE)) {
            cameraMode = CAMERA_FREE;
            camera.up = {0.0f, 1.0f, 0.0f}; // Reset roll
        }

        if (IsKeyPressed(KEY_TWO)) {
            cameraMode = CAMERA_FIRST_PERSON;
            camera.up = {0.0f, 1.0f, 0.0f}; // Reset roll
        }

        if (IsKeyPressed(KEY_THREE)) {
            cameraMode = CAMERA_THIRD_PERSON;
            camera.up = {0.0f, 1.0f, 0.0f}; // Reset roll
        }

        if (IsKeyPressed(KEY_FOUR)) {
            cameraMode = CAMERA_ORBITAL;
            camera.up = {0.0f, 1.0f, 0.0f}; // Reset roll
        }

        // Switch camera projection
        if (IsKeyPressed(KEY_P)) {
            if (camera.projection == CAMERA_PERSPECTIVE) {
                // Create isometric view
                cameraMode = CAMERA_THIRD_PERSON;
                // Note: The target distance is related to the render distance
                // in the orthographic projection
                camera.position = {0.0f, 2.0f, -100.0f};
                camera.target = {0.0f, 0.0f, 0.0f};
                camera.up = {0.0f, 1.0f, 0.0f};
                camera.projection = CAMERA_ORTHOGRAPHIC;
                camera.fovy = 20.0f; // near plane width in CAMERA_ORTHOGRAPHIC
                CameraYaw(&camera, -135 * DEG2RAD, true);
                CameraPitch(&camera, -45 * DEG2RAD, true, true, false);
            } else if (camera.projection == CAMERA_ORTHOGRAPHIC) {
                // Reset to default view
                cameraMode = CAMERA_THIRD_PERSON;
                camera.position = {0.0f, 2.0f, 10.0f};
                camera.target = {0.0f, 2.0f, 0.0f};
                camera.up = {0.0f, 1.0f, 0.0f};
                camera.projection = CAMERA_PERSPECTIVE;
                camera.fovy = 60.0f;
            }
        }

        // Toggle fullscreen
        if (IsKeyPressed(KEY_F11)) {
            fullscreen = !fullscreen;
            ToggleFullscreen();
        }

        if (IsKeyPressed(KEY_X)) {
            chunkManager.genChunk = !chunkManager.genChunk;
        }

        if (IsKeyPressed(KEY_B)) {
            editMode = !editMode;
            if (editMode) {
                EnableCursor();
            } else {
                DisableCursor();
            }
        }

        if (IsKeyPressed(KEY_TAB)) {
            showSettings = !showSettings;
            if (showSettings) {
                EnableCursor();
            } else {
                if (!editMode)
                    DisableCursor();
            }
        }

        // Update camera computes movement internally depending on the camera
        // mode Some default standard keyboard/mouse inputs are hardcoded to
        // simplify use For advanced camera controls, it's recommended to
        // compute camera movement manually
        if (!showSettings) {
            if (!editMode) {
                CustomUpdateCamera(&camera, cameraMode,
                                   movementSpeed); // Update camera
            }
            // deactivate block if clicked, and subsequently update the chunk
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                // get ray mouse ray
                mouseRay = GetScreenToWorldRay(GetMousePosition(), camera);
                // check which chunks the ray hit
                Chunk *chunkHit = nullptr;
                float nearestHitDistance = std::numeric_limits<float>::max();
                // obtain nearest chunk
                for (auto pointChunk : chunkManager.chunks) {
                    Chunk *chunk = pointChunk.second;
                    RayCollision mouseChunkCollision =
                        GetRayCollisionBox(mouseRay, chunk->getBoundingBox());
                    if (mouseChunkCollision.hit) {
                        if (mouseChunkCollision.distance < nearestHitDistance) {
                            chunkHit = chunk;
                            nearestHitDistance = mouseChunkCollision.distance;
                        }
                    }
                }

                if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    // rescramble that chunk to force it to update later
                    if (chunkHit != nullptr) {
                        printf("scramble\n");
                        chunkHit->randomize();
                        chunkManager.QueueChunkToRebuild(chunkHit);
                    }
                }
            }
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(DARKGRAY);

        BeginMode3D(camera);

        // Draw cubes
        // Color col = BLUE;
        // col.a = 100.0f;
        // DrawCube({ 0.0f, 0.0f, 0.0f }, 10.0f, 10.0f, 10.0f, col);     // Draw
        // a blue wall
        DrawCubeWires({0.0f, 0.0f, 0.0f}, 10.0f, 10.0f, 10.0f,
                      BLACK); // Draw a blue wall
        // chunk.render();
        chunkManager.Update(0.0, camera.position, camera.target);
        chunkManager.Render();
        DrawRay(mouseRay, RED);

        // Draw player cube
        if (cameraMode == CAMERA_THIRD_PERSON) {
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
        GuiLabel({15 * scale, 15, 300 * scale, 10 * scale}, "Controls:");
        GuiLabel({15 * scale, 30, 300 * scale, 10 * scale},
                 "- Move keys: W, A, S, D, Space, Left-Ctrl, Edit Mode: B");
        GuiLabel({15 * scale, 45, 300 * scale, 10 * scale},
                 "- Scramble Chunk: L-SHIFT + MOUSE 1");
        GuiLabel({15 * scale, 60, 300 * scale, 10 * scale},
                 "- Look around: arrow keys or mouse");
        GuiLabel({15 * scale, 75, 300 * scale, 10 * scale},
                 "- Camera mode keys: 1, 2, 3, 4");
        GuiLabel({15 * scale, 90, 300 * scale, 10 * scale},
                 "- Zoom keys: num-plus, num-minus or mouse scroll");
        GuiLabel({15 * scale, 105, 300 * scale, 10 * scale},
                 "- Camera projection key: P");
        GuiLabel({15 * scale, 120, 300 * scale, 10 * scale},
                 "- Toggle Chunk Gen: X");
        GuiLabel({15 * scale, 135, 300 * scale, 10 * scale},
                 "- Toggle fullscreen: F11");
        GuiLabel({15 * scale, 150, 300 * scale, 10 * scale},
                 "- Open settings: TAB");
        GuiLabel({15 * scale, 165, 300 * scale, 10 * scale},
                 TextFormat("- FPS: %i", GetFPS()));
        GuiLabel({15 * scale, 180, 300 * scale, 10 * scale},
                 TextFormat("- Frametime: %fms", GetFrameTime() * 1000));
        GuiLabel({15 * scale, 195, 300 * scale, 10 * scale},
                 TextFormat("- Chunk Gen: %s",
                            chunkManager.genChunk ? "On" : "Off"));

        GuiLabel({976 * scale, 15, 300 * scale, 10 * scale}, "Camera status:");
        GuiLabel(
            {976 * scale, 30, 300 * scale, 10 * scale},
            TextFormat("- Mode: %s",
                       (cameraMode == CAMERA_FREE)           ? "FREE"
                       : (cameraMode == CAMERA_FIRST_PERSON) ? "FIRST_PERSON"
                       : (cameraMode == CAMERA_THIRD_PERSON) ? "THIRD_PERSON"
                       : (cameraMode == CAMERA_ORBITAL)      ? "ORBITAL"
                                                             : "CUSTOM"));
        GuiLabel({976 * scale, 45, 300 * scale, 10 * scale},
                 TextFormat("- Projection: %s",
                            (camera.projection == CAMERA_PERSPECTIVE)
                                ? "PERSPECTIVE"
                            : (camera.projection == CAMERA_ORTHOGRAPHIC)
                                ? "ORTHOGRAPHIC"
                                : "CUSTOM"));
        GuiLabel({976 * scale, 60, 300 * scale, 10 * scale},
                 TextFormat("- Position: (%06.3f, %06.3f, %06.3f)",
                            camera.position.x, camera.position.y,
                            camera.position.z));
        GuiLabel({976 * scale, 75, 300 * scale, 10 * scale},
                 TextFormat("- Target: (%06.3f, %06.3f, %06.3f)",
                            camera.target.x, camera.target.y, camera.target.z));
        GuiLabel({976 * scale, 90, 300 * scale, 10 * scale},
                 TextFormat("- Up: (%06.3f, %06.3f, %06.3f)", camera.up.x,
                            camera.up.y, camera.up.z));

        // show settings
        if (showSettings)
            GuiSlider({100 * scale, 500, 300 * scale, 10 * scale},
                      TextFormat("Move Speed: %0.2f", movementSpeed), NULL,
                      &movementSpeed, 0.01f, 10.0f);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
