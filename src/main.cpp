#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <learnopengl/shader_m.h>

#include "Chunk.h"
#include "ChunkManager.h"
#include "smolgl.h"
#include <iostream>
#include <unordered_map>
#include <glfw/src/internal.h>
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

ChunkManager chunkManager;
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
    Shader *ourShader =
        new Shader("src/shaders/camera.vert", "src/shaders/camera.frag");

    // glm::vec3 pos = glm::vec3(0, 0, 0);
    // Chunk chunk = Chunk(pos, ourShader);
    // chunk.load();
    // chunk.setup();
    chunkManager = ChunkManager(4, 3, ourShader);

    char fpsStr[32] = "FPS: 0";
    char memStr[32];

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

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

        // chunk.render();
        chunkManager.update(deltaTime, cameraPos, cameraPos + cameraFront);
        chunkManager.render();

        // Rendering
        // (Your code clears your framebuffer, renders your other stuff etc.)
        // (Your code calls glfwSwapBuffers() etc.)

        // Calculate  FPS
        int fps = calculateFPS(deltaTime);
        float mem = calculateMemUsage();
        if (fps != -1) {
            std::sprintf(fpsStr, "FPS: %d", fps);
        }
        std::sprintf(memStr, "RAM: %f MB", mem / 1000000);

        ImGui::Begin("Stats");
        // if(DEBUG){
        // 	ImGui::DragInt("tps", &g.ticksPerSecond, 1, 0, 1000);
        // }
        // Text that appears in the window
        // ImGui::Text("Hello there adventurer!");
        ImGui::Text("%s", fpsStr);
        ImGui::Text("%s", memStr);

        // Slider that appears in the window
        // Ends the window
        ImGui::End();

        ImGui::Begin("Camera");
        ImGui::Text("fov: %.2f", fov);
        ImGui::Text("pos: (%.2f, %.3f, %.3f)", cameraPos.x, cameraPos.y,
                    cameraPos.z);
        ImGui::Text("left: (%.2f, %.3f, %.3f)", cameraLeft.x, cameraLeft.y,
                    cameraLeft.z);
        ImGui::Text("right: (%.2f, %.3f, %.3f)", cameraRight.x, cameraRight.y,
                    cameraRight.z);
        ImGui::Text("up: (%.2f, %.3f, %.3f)", cameraUp.x, cameraUp.y,
                    cameraUp.z);
        ImGui::Text("frustum:");
        ImGui::Text("left d = %.2f",
                    frustum.planes[Frustum::FRUSTUM_LEFT].distance);
        ImGui::Text("right d = %.2f",
                    frustum.planes[Frustum::FRUSTUM_RIGHT].distance);
        ImGui::Text("left: n:(%.2f, %.3f, %.3f)\nright: n:(%.3f, %.3f, %.3f)",
                    frustum.planes[Frustum::FRUSTUM_LEFT].normal.x,
                    frustum.planes[Frustum::FRUSTUM_LEFT].normal.y,
                    frustum.planes[Frustum::FRUSTUM_LEFT].normal.z,
                    frustum.planes[Frustum::FRUSTUM_RIGHT].normal.x,
                    frustum.planes[Frustum::FRUSTUM_RIGHT].normal.y,
                    frustum.planes[Frustum::FRUSTUM_RIGHT].normal.z);

        ImGui::Text("near: n:(%.2f, %.3f, %.3f)\nfar: n:(%.3f, %.3f, %.3f)",
                    frustum.planes[Frustum::FRUSTUM_NEAR].normal.x,
                    frustum.planes[Frustum::FRUSTUM_NEAR].normal.y,
                    frustum.planes[Frustum::FRUSTUM_NEAR].normal.z,
                    frustum.planes[Frustum::FRUSTUM_FAR].normal.x,
                    frustum.planes[Frustum::FRUSTUM_FAR].normal.y,
                    frustum.planes[Frustum::FRUSTUM_FAR].normal.z);

        ImGui::Text("bottom: n:(%.2f, %.3f, %.3f)\ntop: n:(%.3f, %.3f, %.3f)",
                    frustum.planes[Frustum::FRUSTUM_BOTTOM].normal.x,
                    frustum.planes[Frustum::FRUSTUM_BOTTOM].normal.y,
                    frustum.planes[Frustum::FRUSTUM_BOTTOM].normal.z,
                    frustum.planes[Frustum::FRUSTUM_TOP].normal.x,
                    frustum.planes[Frustum::FRUSTUM_TOP].normal.y,
                    frustum.planes[Frustum::FRUSTUM_TOP].normal.z);
        ImGui::End();

        if (cursorOn) {
            ImGui::Begin("Debug Menu");
            // if(DEBUG){
            // 	ImGui::DragInt("tps", &g.ticksPerSecond, 1, 0, 1000);
            // }
            // Text that appears in the window
            ImGui::Checkbox("generate chunks", &chunkManager.genChunk);
            ImGui::LabelText("##moveSpeedLabel", "Movement Speed");
            ImGui::SliderFloat("##moveSpeedSlider", &cameraSpeedMultiplier,
                               1.0f, 1000.0f);
            ImGui::LabelText("##chunkGenDistanceLabel", "Chunk Gen Distance");
            ImGui::SliderInt("##chunkGenDistanceSlider",
                             (int *)&chunkManager.chunkGenDistance, 1, 16);
            ImGui::LabelText("##renderDistanceLabel", "Render Distance");
            ImGui::SliderInt("##renderDistanceSlider",
                             (int *)&chunkManager.chunkRenderDistance, 1, 16);
            ImGui::LabelText("##zFarLabel", "zFar");
            ImGui::SliderFloat("##zFarSlider", &zFar, 1.0f, 2000.0f);
            ImGui::LabelText("##fovSliderLabel", "FOV");
            ImGui::SliderFloat("##fovSlider", &fov, 25.0f, 105.0f);
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

    float cameraSpeed = static_cast<float>(cameraSpeedMultiplier * deltaTime);
    constexpr glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -=
            glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos +=
            glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos +=  up * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cameraPos -= up * cameraSpeed;
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
        chunkManager.genChunk = !chunkManager.genChunk;
        keyPressMap[GLFW_KEY_X] = false;
    }

    frustum = createFrustumFromCamera(SCR_WIDTH / SCR_HEIGHT, fov, zNear, zFar);
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

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset =
        lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get
    // flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(front);

    // Calculate the right vector
    cameraRight =
        glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));

    // Calculate the up vector
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));

    // Calculate the left vector (opposite of right)
    cameraLeft = -cameraRight;

    // The top vector is the same as the up vector in this case
    cameraTop = cameraUp;

    frustum = createFrustumFromCamera(SCR_WIDTH / SCR_HEIGHT, fov, zNear, zFar);
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
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 105.0f)
        fov = 105.0f;

    frustum = createFrustumFromCamera(SCR_WIDTH / SCR_HEIGHT, fov, zNear, zFar);
}
