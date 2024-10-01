#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <learnopengl/shader_m.h>

#include "Ecs.h"
#include "PhysicsSystem.h"
#include "Mesh.h"

#include <iostream>
#include "utils.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void imgui_mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, bool *cursorOn);
int calculateFPS(float deltaTime);
float calculateMemUsage();
void renderText(Shader &shader, std::string text, float x, float y, float scale,
                glm::vec3 color);

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

// key press map
std::unordered_map<int, bool> keyPressMap;

// Terrain chunk manager
ChunkManager *chunkManager;
// Global coordinator
Coordinator gCoordinator;

const int FPS_HISTORY_CAP = 5000;
const int MEM_HISTORY_CAP = 5000;
std::vector<float> fpsHistory;
std::vector<float> memHistory;

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "woksol", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    // imgui!!!
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // IF using Docking
    // Branch

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(
        window, true); // Second param install_callback=true will install GLFW
                       // callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init(nullptr);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    bool cursorOn = false;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile shader programs
    // ------------------------------------
    // TODO: should we abstract these away and just move them into smolgl.h?
    Shader *ourShader =
        new Shader("src/shaders/terrain.vert", "src/shaders/terrain.frag");
    Shader *defaultShader =
        new Shader("src/shaders/shader.vert", "src/shaders/shader.frag");

    // glm::vec3 pos = glm::vec3(0, 0, 0);
    // Chunk chunk = Chunk(pos, ourShader);
    // chunk.load();
    // chunk.setup();

    char fpsStr[32] = "FPS: 0";
    char memStr[32];

    // initialize coordinator
    chunkManager = new ChunkManager(4, 3, ourShader);
    gCoordinator.Init(chunkManager);

    // generate terrain
    gCoordinator.mChunkManager->pregenerateChunks();

    gCoordinator.RegisterComponent<Gravity>();
    gCoordinator.RegisterComponent<RigidBody>();
    gCoordinator.RegisterComponent<Transform>();
    gCoordinator.RegisterComponent<Mesh>();

    auto physicsSystem = gCoordinator.RegisterSystem<PhysicsSystem>();

    Signature signature;
    signature.set(gCoordinator.GetComponentType<Gravity>());
    signature.set(gCoordinator.GetComponentType<RigidBody>());
    signature.set(gCoordinator.GetComponentType<Transform>());
    signature.set(gCoordinator.GetComponentType<Mesh>());
    gCoordinator.SetSystemSignature<PhysicsSystem>(signature);

    std::vector<Entity> entities(MAX_ENTITIES);

    // create a dummy "player entity"
    entities[0] = gCoordinator.CreateEntity();
    gCoordinator.AddComponent(entities[0],
                              Gravity{glm::vec3(0.0f, -0.1f, 0.0f)});
    gCoordinator.AddComponent(
        entities[0],
        RigidBody{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)});

    // place the entity in front of us
    gCoordinator.AddComponent(
        entities[0], Transform{.position = glm::vec3(0.0f, 4.0f, -5.0f),
                               .rotation = glm::vec3(0.0f, 0.0f, 0.0f),
                               .scale = glm::vec3(1.0f, 1.0f, 1.0f)});

    // create cube mesh
    glm::vec3 vertices[] = {
        // Front face
        glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec3(1.0f, -1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 1.0f, 1.0f),

        // Back face
        glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, -1.0f, -1.0f),
        glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(-1.0f, 1.0f, -1.0f)};

    unsigned int indices[] = {// Front face
                              0, 1, 2, 2, 3, 0,
                              // Back face
                              4, 5, 6, 6, 7, 4,
                              // Left face
                              4, 0, 3, 3, 7, 4,
                              // Right face
                              1, 5, 6, 6, 2, 1,
                              // Top face
                              3, 2, 6, 6, 7, 3,
                              // Bottom face
                              4, 5, 1, 1, 0, 4};

    int numVertices = 8;
    int numTriangles = 16;
    int sizeIndicies = 36;

    Mesh mesh = Mesh{.vertices = &vertices[0],
                     .sizeVertices = numVertices,
                     .numTriangles = numTriangles,
                     .indices = &indices[0],
                     .sizeIndices = sizeIndicies,
                     .color = {1.0f, 0.0f, 0.0f, 1.0f}
                    };
    
    // create material with default shader
    Material defaultMaterial = Material(defaultShader);

    // TODO: create a "MeshManager"
    UploadMesh(&mesh, true);

    gCoordinator.AddComponent(
        entities[0], mesh);

    auto player = entities[0];

    // render loop
    // -----------

    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        physicsSystem->Update(deltaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // input
        // -----
        processInput(window, &cursorOn);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // update
        gCoordinator.mChunkManager->update(deltaTime, gCoordinator.mCamera);
        // get player deets
        RigidBody playerRB = gCoordinator.GetComponent<RigidBody>(player);
        Transform playerTrans = gCoordinator.GetComponent<Transform>(player);

        // render
        gCoordinator.mChunkManager->render(gCoordinator.mCamera);
        DrawMesh(gCoordinator.mCamera, mesh, defaultMaterial, playerTrans.position);

        // TODO: render the "player" entity
        // defaultShader->use();
        // glUseProgram(0);

        // Calculate  FPS
        int fps = calculateFPS(deltaTime);
        float mem = calculateMemUsage();
        if (fps != -1) {
            std::sprintf(fpsStr, "FPS: %d", fps);
        }
        std::sprintf(memStr, "RAM: %f MB", mem / 1000000);

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always,
                                ImVec2(0.0f, 0.0f));
        ImGuiWindowFlags statsFlags =
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        statsFlags |= ImGuiWindowFlags_NoMove;
        bool active = true;
        ImGui::Begin("Stats", &active, statsFlags);
        ImGui::Text("%s", fpsStr);
        ImGui::Text("%s", memStr);
        ImGui::Separator();
        // Ends the window
        ImGui::End();

        ImGui::Begin("Player");
        ImGui::Text("velocity: (%.2f, %.3f, %.3f)", playerRB.velocity.x,
                    playerRB.velocity.y, playerRB.velocity.z);
        ImGui::Text("position: (%.2f, %.3f, %.3f)", playerTrans.position.x,
                    playerTrans.position.y, playerTrans.position.z);
        ImGui::End();

        ImGui::Begin("Camera");
        ImGui::Text("fov: %.2f", gCoordinator.mCamera.fov);
        ImGui::Text("pos: (%.2f, %.3f, %.3f)", gCoordinator.mCamera.cameraPos.x,
                    gCoordinator.mCamera.cameraPos.y,
                    gCoordinator.mCamera.cameraPos.z);
        ImGui::Text("left: (%.2f, %.3f, %.3f)",
                    gCoordinator.mCamera.cameraLeft.x,
                    gCoordinator.mCamera.cameraLeft.y,
                    gCoordinator.mCamera.cameraLeft.z);
        ImGui::Text("right: (%.2f, %.3f, %.3f)",
                    gCoordinator.mCamera.cameraRight.x,
                    gCoordinator.mCamera.cameraRight.y,
                    gCoordinator.mCamera.cameraRight.z);
        ImGui::Text("up: (%.2f, %.3f, %.3f)", gCoordinator.mCamera.cameraUp.x,
                    gCoordinator.mCamera.cameraUp.y,
                    gCoordinator.mCamera.cameraUp.z);
        ImGui::Text("frustum:");
        ImGui::Text("left d = %.2f",
                    gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_LEFT]
                        .distance);
        ImGui::Text("right d = %.2f",
                    gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_RIGHT]
                        .distance);
        ImGui::Text(
            "left: n:(%.2f, %.3f, %.3f)\nright: n:(%.3f, %.3f, %.3f)",
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_LEFT].normal.x,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_LEFT].normal.y,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_LEFT].normal.z,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_RIGHT]
                .normal.x,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_RIGHT]
                .normal.y,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_RIGHT]
                .normal.z);

        ImGui::Text(
            "near: n:(%.2f, %.3f, %.3f)\nfar: n:(%.3f, %.3f, %.3f)",
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_NEAR].normal.x,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_NEAR].normal.y,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_NEAR].normal.z,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_FAR].normal.x,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_FAR].normal.y,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_FAR].normal.z);

        ImGui::Text(
            "bottom: n:(%.2f, %.3f, %.3f)\ntop: n:(%.3f, %.3f, %.3f)",
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_BOTTOM]
                .normal.x,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_BOTTOM]
                .normal.y,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_BOTTOM]
                .normal.z,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_TOP].normal.x,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_TOP].normal.y,
            gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_TOP].normal.z);
        ImGui::End();

        if (cursorOn) {
            ImGui::Begin("Debug Menu");
            // if(DEBUG){
            // 	ImGui::DragInt("tps", &g.ticksPerSecond, 1, 0, 1000);
            // }
            // Text that appears in the window
            ImGui::Checkbox("generate chunks",
                            &gCoordinator.mChunkManager->genChunk);
            ImGui::LabelText("##moveSpeedLabel", "Movement Speed");
            ImGui::SliderFloat("##moveSpeedSlider",
                               &gCoordinator.mCamera.cameraSpeedMultiplier,
                               1.0f, 1000.0f);
            ImGui::LabelText("##chunkGenDistanceLabel", "Chunk Gen Distance");
            ImGui::SliderInt(
                "##chunkGenDistanceSlider",
                (int *)&(gCoordinator.mChunkManager->chunkGenDistance), 1, 16);
            ImGui::LabelText("##renderDistanceLabel", "Render Distance");
            ImGui::SliderInt(
                "##renderDistanceSlider",
                (int *)&(gCoordinator.mChunkManager->chunkRenderDistance), 1,
                16);
            ImGui::LabelText("##zFarLabel", "zFar");
            ImGui::SliderFloat("##zFarSlider", &gCoordinator.mCamera.zFar, 1.0f,
                               2000.0f);
            ImGui::LabelText("##fovSliderLabel", "FOV");
            ImGui::SliderFloat("##fovSlider", &gCoordinator.mCamera.fov, 25.0f,
                               105.0f);
            // Slider that appears in the window
            // Ends the window
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, bool *cursorOn) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(
        gCoordinator.mCamera.cameraSpeedMultiplier * deltaTime);
    constexpr glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCoordinator.mCamera.cameraPos +=
            cameraSpeed * gCoordinator.mCamera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCoordinator.mCamera.cameraPos -=
            cameraSpeed * gCoordinator.mCamera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCoordinator.mCamera.cameraPos -=
            glm::normalize(glm::cross(gCoordinator.mCamera.cameraFront,
                                      gCoordinator.mCamera.cameraUp)) *
            cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCoordinator.mCamera.cameraPos +=
            glm::normalize(glm::cross(gCoordinator.mCamera.cameraFront,
                                      gCoordinator.mCamera.cameraUp)) *
            cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        gCoordinator.mCamera.cameraPos += up * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        gCoordinator.mCamera.cameraPos -= up * cameraSpeed;
    }

    // insert into key press map
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        keyPressMap[GLFW_KEY_B] = true;
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        keyPressMap[GLFW_KEY_X] = true;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE &&
        keyPressMap[GLFW_KEY_B]) {
        *cursorOn = !(*cursorOn);
        glfwSetCursorPosCallback(window, *cursorOn ? imgui_mouse_callback
                                                   : mouse_callback);
        glfwSetScrollCallback(window, *cursorOn ? imgui_mouse_callback
                                                : scroll_callback);
        glfwSetInputMode(window, GLFW_CURSOR,
                         *cursorOn ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        keyPressMap[GLFW_KEY_B] = false;
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE &&
        keyPressMap[GLFW_KEY_X]) {
        gCoordinator.mChunkManager->genChunk =
            !gCoordinator.mChunkManager->genChunk;
        keyPressMap[GLFW_KEY_X] = false;
    }

    // TODO: a better way to do this?
    gCoordinator.mCamera.frustum =
        createFrustumFromCamera(gCoordinator.mCamera);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, width, height);
}

// Function to calculate and return the FPS as a string
int calculateFPS(float deltaTime) {
    static int frameCount = 0;
    static float elapsedTime = 0.0f;
    static float lastTime = 0.0f;

    elapsedTime += deltaTime;
    frameCount++;

    if (elapsedTime - lastTime >= 1.0f) { // Update every second
        lastTime = elapsedTime;
        int fps = frameCount;
        frameCount = 0;
        return fps;
    }

    return -1.0;
}

// Function to calculate and return the RAM usage as a string
float calculateMemUsage() {
    float memUsage = (float)getMemoryUsage();
    return memUsage;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (gCoordinator.mCamera.firstMouse) {
        gCoordinator.mCamera.lastX = xpos;
        gCoordinator.mCamera.lastY = ypos;
        gCoordinator.mCamera.firstMouse = false;
    }

    float xoffset = xpos - gCoordinator.mCamera.lastX;
    float yoffset = gCoordinator.mCamera.lastY -
                    ypos; // reversed since y-coordinates go from bottom to top
    gCoordinator.mCamera.lastX = xpos;
    gCoordinator.mCamera.lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    gCoordinator.mCamera.yaw += xoffset;
    gCoordinator.mCamera.pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get
    // flipped
    if (gCoordinator.mCamera.pitch > 89.0f)
        gCoordinator.mCamera.pitch = 89.0f;
    if (gCoordinator.mCamera.pitch < -89.0f)
        gCoordinator.mCamera.pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(gCoordinator.mCamera.yaw)) *
              cos(glm::radians(gCoordinator.mCamera.pitch));
    front.y = sin(glm::radians(gCoordinator.mCamera.pitch));
    front.z = sin(glm::radians(gCoordinator.mCamera.yaw)) *
              cos(glm::radians(gCoordinator.mCamera.pitch));

    gCoordinator.mCamera.cameraFront = glm::normalize(front);

    // Calculate the right vector
    gCoordinator.mCamera.cameraRight = glm::normalize(glm::cross(
        gCoordinator.mCamera.cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));

    // Calculate the up vector
    gCoordinator.mCamera.cameraUp = glm::normalize(glm::cross(
        gCoordinator.mCamera.cameraRight, gCoordinator.mCamera.cameraFront));

    // Calculate the left vector (opposite of right)
    gCoordinator.mCamera.cameraLeft = -gCoordinator.mCamera.cameraRight;

    // The top vector is the same as the up vector in this case
    gCoordinator.mCamera.cameraTop = gCoordinator.mCamera.cameraUp;

    gCoordinator.mCamera.frustum =
        createFrustumFromCamera(gCoordinator.mCamera);
}

void imgui_mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    ImGuiIO io = ImGui::GetIO();
    double cursorX, cursorY;
    glfwGetCursorPos(window, &cursorX, &cursorY);
    ImGui_ImplGlfw_CursorPosCallback(window, cursorX, cursorY);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    gCoordinator.mCamera.cameraSpeedMultiplier -= (float)yoffset;
    if (gCoordinator.mCamera.cameraSpeedMultiplier < 1.0f)
        gCoordinator.mCamera.cameraSpeedMultiplier = 1.0f;
    if (gCoordinator.mCamera.cameraSpeedMultiplier > 1000.0f)
        gCoordinator.mCamera.cameraSpeedMultiplier = 1000.0f;

    gCoordinator.mCamera.frustum =
        createFrustumFromCamera(gCoordinator.mCamera);
}
