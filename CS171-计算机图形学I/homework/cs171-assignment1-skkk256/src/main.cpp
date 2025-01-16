#include <mesh.h>
#include <utils.h>
#include <vector>
#include "loadShader.hpp"
#include "shader.h"
#include "camera.h"

const bool USE_GEOM = true;

const int kWidth = 1200;
const int kHeight = 900;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = kWidth / 2.0f;
float lastY = kHeight / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 10.0f);
glm::vec3 lightPos2(1.2f, 1.0f, -5.0f);

/// Process the input events
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

///
/// \todo Insert lines at where you think it's right and add the necessaries to
/// draw the mesh onto the screen, e.g.,
/// - Setup the shader program with functions encapsulated inside the \see
/// Shader class
/// - Setup and bind the geometry with \see Mesh class
/// - Inside the <em>main loop</em>, draw the mesh with \see Mesh::draw
///
int main() {
  // Claim the window resource and its OpenGL-related context.
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // glfw window creation
  // --------------------
  GLFWwindow* window = glfwCreateWindow(kWidth, kHeight, "CS171", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  // Enable depth test
  // - https://www.khronos.org/opengl/wiki/Depth_Test
  // - https://learnopengl.com/Advanced-OpenGL/Depth-testing
  glEnable(GL_DEPTH_TEST);
  Shader normalShader("../include/normal_visualization.vert", "../include/normal_visualization.frag", "../include/normal_visualization.geom");
  std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
  Shader shader("../include/SimpleVertexShader.vert", "../include/SimpleFragmentShader.frag", "");


  //  GLuint programID = LoadShaders( "../include/SimpleVertexShader.vert", "../include/SimpleFragmentShader.frag" );
  GLuint shaderProgram = shader.getID();
  Mesh mesh("assets/bunny.object");
  // The main loop of the GLFW window
  while (!glfwWindowShouldClose(window)){
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    // Input
    // -----
    processInput(window);

    // Render
    // ------
    glClearColor(0.1F, 0.2F, 0.3F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    shader.use();
    // update the uniform color
    shader.setVec3("objectColor", vec3{1.0f, 0.5f, 0.31f});
    shader.setVec3("lightColor",  vec3{1.0f, 1.0f, 1.0f});
    shader.setVec3("lightPos", lightPos);
    shader.setVec3("lightColor2",  vec3{0.1f, 0.7f, 0.31f});
    shader.setVec3("lightPos2", lightPos2);
    shader.setVec3("viewPos", camera.Position);


    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)kWidth / (float)kHeight, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    // world transformation
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    // Draw object
    mesh.draw();
    // then draw model with normal visualizing geometry shader
    if (USE_GEOM) {
      normalShader.use();
      normalShader.setMat4("projection", projection);
      normalShader.setMat4("view", view);
      normalShader.setMat4("model", model);

      // Draw object
      mesh.draw();
    }
    //    std::cout << glGetError() << std::endl;
    // GLFW
    // ----
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  // glfw: terminate, clearing all previously allocated GLFW resources.
  // ------------------------------------------------------------------
  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (firstMouse)
  {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}