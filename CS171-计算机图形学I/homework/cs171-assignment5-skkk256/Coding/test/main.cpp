#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "cloth_renderer.hpp"
#include "cloth_simulator.hpp"
#include "mesh.h"

void processCameraInput(GLFWwindow* window, FirstPersonCamera* camera);

int main(int argc, char* argv[])
{
    GLFWwindow* window;
    // Window setup
    {
        if (!glfwInit()) // Initialize glfw library
            return -1;

        // setting glfw window hints and global configurations
        {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Use Core Mode
            // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // Use Debug Context
        #ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // fix compilation on OS X
        #endif
        }

        // Create a windowed mode window and its OpenGL context
        window = glfwCreateWindow(1000, 1000, "Cloth Simulation with Mass Spring", NULL, NULL);
        if (!window) {
            glfwTerminate();
            return -1;
        }

        // window configurations
        {
            // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        // Make the window's context current
        glfwMakeContextCurrent(window);

        // Load Opengl
        if (!gladLoadGL()) {
            glfwTerminate();
            return -1;
        };

        glEnable(GL_DEPTH_TEST);
    }

    // Main Loop
    {
        // TODO: Tune overall and cloth settings here (especially shader path)

        // Overall settings
        auto vertexShader = "res/shader/cloth.vs";
        auto fragmentShader = "res/shader/cloth.fs";
        const float timeStep = 0.0002f;

        // Cloth settings
        unsigned int nWidth = 40;
        unsigned int nHeight = 30;
        float dx = 0.1f;
        auto clothTransform = glm::rotate(glm::mat4(1.0f),
                                          glm::radians(60.0f), {1.0f, 0.0f, 0.0f}); // Represents a rotation of 60 degrees around the x-axis.
        float totalMass = 1.0f;
        float stiffnessReference = 40.0f;
        float airResistanceCoefficient = 0.001f;
        glm::vec3 gravity = {0.0f, -9.81f, 0.0f};
//        glm::vec3 wind = {0.002f, 0.005f, 0.01f};
        glm::vec3 wind = {0.0f, 0.0f, 0.0f};

        // Create objects
        Shader shader(vertexShader, fragmentShader);
        FirstPersonCamera camera;
        RectCloth cloth(nWidth, nHeight, dx, clothTransform);
        Mesh mesh(glm::vec3(0, -2, 0), 0.8);
        RectClothRenderer renderer(&shader, &camera, &cloth, &mesh);
        RectClothSimulator simulator(&cloth, &mesh, totalMass, stiffnessReference, airResistanceCoefficient, gravity, wind);

        // Setup iteration variables
        float currentTime = (float)glfwGetTime();
        float lastTime = currentTime;
        float deltaTime = 0.0f;

        int totalIterCount = 0;
        float totalIterTime = 0.0f;
        float overTakenTime = 0.0f;

        // Loop until the user closes the window
        while (!glfwWindowShouldClose(window))
        {
            // Terminate condition
            if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);

            // Updating
            {
                // Calculate dt
                currentTime = static_cast<float>(glfwGetTime());
                deltaTime = currentTime - lastTime - overTakenTime; // maintain the deltaTime not get too large
                overTakenTime = 0.0f;
                lastTime = currentTime;

                processCameraInput(window, &camera);

                // Debug Update here only when p is pressed
                if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
                {
                    // A fixed time step which should not be too large in order to stabilize the simulation
                    totalIterTime += deltaTime;
                    float curIterTime = totalIterTime - (float)totalIterCount * timeStep;
                    int iterCount = (int)roundf(curIterTime / timeStep);

                    for (int i = 0; i < iterCount; ++i) {
                        totalIterCount += 1;

                        // Simulate one step
                        simulator.step(timeStep);
                        if(glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS){
                            simulator.movePoint(UP);
                        }
                        else if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
                            simulator.movePoint(FORWARD);
                        }
                        else if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
                            simulator.movePoint(RIGHT);
                        }

                        float timeTaken = static_cast<float>(glfwGetTime()) - currentTime;
                        if (timeTaken > deltaTime) {
                            overTakenTime = timeTaken - deltaTime;
                            break;
                        }
                    }
                }
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Draw here
            renderer.draw();

            // Swap front and back buffers
            glfwSwapBuffers(window);

            // Poll for and process events
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}

void processCameraInput(GLFWwindow* window, FirstPersonCamera* camera)
{
    static bool firstRun {true};
    static float lastFrame {0};

    static float lastCursorX {0};
    static float lastCursorY {0};

    double curCursorX, curCursorY;
    glfwGetCursorPos(window, &curCursorX, &curCursorY);

    float currentFrame = static_cast<float>(glfwGetTime());

    if (firstRun) {
        lastFrame = currentFrame;
        firstRun = false;

        lastCursorX = static_cast<float>(curCursorX);
        lastCursorY = static_cast<float>(curCursorY);

        return; // everything zero, so we return directly
    }

    float deltaTime = currentFrame - lastFrame;
    float deltaCursorX = curCursorX - lastCursorX;
    float deltaCursorY = curCursorY - lastCursorY;

    float cameraMoveSpeed = 3.5f * deltaTime;
    float cameraRotateSpeed = 1.0f * deltaTime;

	if((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_S) != GLFW_PRESS))
        camera->moveForward(cameraMoveSpeed);
    if((glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_W) != GLFW_PRESS))
        camera->moveBackward(cameraMoveSpeed);
    if((glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_A) != GLFW_PRESS))
        camera->moveRight(cameraMoveSpeed);
    if((glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_D) != GLFW_PRESS))
        camera->moveLeft(cameraMoveSpeed);
    if((glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS))
        camera->moveUp(cameraMoveSpeed);
    if((glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS))
        camera->moveDown(cameraMoveSpeed);

    if((glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_PRESS))
        camera->lookRight(cameraRotateSpeed);
    if((glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_PRESS))
        camera->lookLeft(cameraRotateSpeed);
    if((glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_DOWN) != GLFW_PRESS))
        camera->lookUp(cameraRotateSpeed);
    if((glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_UP) != GLFW_PRESS))
        camera->lookDown(cameraRotateSpeed);
    if((glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_E) != GLFW_PRESS))
        camera->rotateLeft(cameraRotateSpeed);
    if((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS))
        camera->rotateRight(cameraRotateSpeed);

     camera->lookLeft(cameraRotateSpeed * deltaCursorX * 0.2f);
     camera->lookDown(cameraRotateSpeed * deltaCursorY * 0.2f);

    // update record
    lastCursorX = static_cast<float>(curCursorX);
    lastCursorY = static_cast<float>(curCursorY);

    lastFrame = currentFrame;
}
