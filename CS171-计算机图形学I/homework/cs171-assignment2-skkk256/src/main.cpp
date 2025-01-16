#include <utils.h>
#include <object.h>
#include <shader.h>
#include <bezier.h>
#include <camera.h>
#include "data.h"
#include "bspline.h"

// callback functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double x, double y);

// util functions
Object generateSphere(vec3 center, float radius, int resolution);
glm::vec2 WorldToScreen(const glm::vec3& worldPos, const glm::mat4& projection, const glm::mat4& view, int viewportWidth, int viewportHeight);
glm::vec3 ScreenToWorld(float x, float y, const glm::mat4& projection, const glm::mat4& view, int viewportWidth, int viewportHeight);
float distance(const vec3& a, const vec3& b);

// running
void run_patch(Shader shader, Camera mycamera);
void run_interaction(Shader shader, Camera mycamera);
void run_bspline(Shader shader, Camera mycamera);


// settings
enum mode {
    INTERACTION,
    PATCH,
    BSPLINE,
};
mode current_mode = INTERACTION;
const int WIDTH = 800;
const int HEIGHT = 600;
glm::vec3 lightPos(0.0f, 7.0f, 0.0f);

bool selected_flag = false;
vec2 selected_point{0, 0};
bool firstMouse = true;
float lastX = WIDTH/2.0;
float lastY = HEIGHT/2.0;

GLFWwindow* window;
Camera mycamera;


int main() {
  WindowGuard windowGuard(window, WIDTH, HEIGHT, "CS171 hw2");
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR_NORMAL, GLFW_CURSOR_DISABLED);
  glEnable(GL_DEPTH_TEST);

  Shader shader("../include/shader.vert", "../include/shader.frag");
  Shader selection_shader("../include/SimpleShader.vert", "../include/SimpleShader.frag");

  Object mouse_obj;
  mouse_obj.setData(mouse);
  mouse_obj.setDrawMode(GL_TRIANGLES);

  
  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    if (current_mode == INTERACTION) {
      run_interaction(shader, mycamera);
    } else if (current_mode == PATCH) {
      run_patch(shader, mycamera);
    } else if (current_mode == BSPLINE) {
      run_bspline(shader, mycamera);
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
      selection_shader.use();
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);
      selection_shader.setVec2("mousePos", vec2{2 * xpos, 2 * (HEIGHT - ypos)});
      mouse_obj.draw();
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  return 0;
}

void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    mycamera.processWalkAround(Forward);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    mycamera.processWalkAround(Backward);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    mycamera.processWalkAround(Leftward);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    mycamera.processWalkAround(Rightward);
  }
  if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
    current_mode = INTERACTION;
  }
  if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
    current_mode = PATCH;
  }
  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
    current_mode = BSPLINE;
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    std::cout << current_mode << "\n";
    std::cout << glGetError() << std::endl;
    std::cout << "Camera position: (" << mycamera.Position.x << ", "
         << mycamera.Position.y << ", " << mycamera.Position.z << ")" << std::endl;
  }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double x, double y) {
  x = (float)x;
  y = (float)y;
  if (firstMouse) {
    lastX = x;
    lastY = y;
    firstMouse = false;
  }
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
    lastX = x;
    lastY = y;
    return;
  }
  else {
    mycamera.processLookAround(x - lastX, lastY - y);
    // update record
    lastX = x;
    lastY = y;
  }
}

float distance(const vec2& a, const vec2& b) {
  float dx = a.x - b.x;
  float dy = a.y - b.y;
  return std::sqrt(dx * dx + dy * dy);
}

Object generateSphere(vec3 center, float radius, int resolution) {
  Object obj;
  std::vector<Vertex> vertices{};
  std::vector<GLuint> indices;
  float step = 1.0 / resolution; // resolution is the number of points you want to generate on the surface in each direction
  for (float u = 0; u <= 1; u += step) {
    for (float v = 0; v <= 1; v += step) {
      Vertex vert;
      vert.position = center + radius * vec3{sin(u * 2 * M_PI) * cos(v * M_PI), sin(u * 2 * M_PI) * sin(v * M_PI), cos(u * 2 * M_PI)};
      vert.normal = glm::normalize(vert.position - center);
      vertices.push_back(vert);
    }
  }
  GLuint length = pow(vertices.size(), 0.5);

  for(int i = 0; i < length - 1; i++) {
    for(int j = 0; j < length - 1; j++) {
      indices.push_back(i * length + j);
      indices.push_back(i * length + j + 1);
      indices.push_back((i + 1) * length + j);
      indices.push_back((i + 1) * length + j);
      indices.push_back(i * length + j + 1);
      indices.push_back((i + 1) * length + j + 1);
    }
  }
  obj.setData(vertices, indices);
  return obj;
}

glm::vec2 WorldToScreen(const glm::vec3& worldPos, const glm::mat4& projection, const glm::mat4& view, int viewportWidth, int viewportHeight) {
  // Transform to clip space
  glm::vec4 clipSpacePos = projection * view * glm::vec4(worldPos, 1.0f);

  // Perform perspective divide
  glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos) / clipSpacePos.w;

  // Transform to screen space
  glm::vec2 screenSpacePos = ((glm::vec2(ndcSpacePos) + glm::vec2(1.0f)) / 2.0f) * glm::vec2(viewportWidth, viewportHeight);

  screenSpacePos.y = viewportHeight - screenSpacePos.y;

  return screenSpacePos;
}

glm::vec3 ScreenToWorld(float x, float y, const glm::mat4& projection, const glm::mat4& view, int viewportWidth, int viewportHeight) {
  // Transform to normalized device coordinates
  glm::vec3 ndcSpacePos;
  ndcSpacePos.x = ((2.0f * x) / viewportWidth) - 1.0f;
  ndcSpacePos.y = 1.0f - ((2.0f * y) / viewportHeight);

  // Get depth value from depth buffer
  glReadPixels(x, viewportHeight - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &ndcSpacePos.z);

  // Transform to clip space
  float clipW = projection[2][3] / (ndcSpacePos.z - (projection[2][2] / projection[3][2]));
  glm::vec4 clipSpacePos = vec4(ndcSpacePos * clipW, clipW);

  // Transform to world space
  glm::vec4 inverseTransform = glm::inverse(projection) * clipSpacePos;
  glm::vec4 worldSpacePos = glm::inverse(view) * inverseTransform;

//  std::cout << ndcSpacePos.x /worldSpacePos.w << " " << ndcSpacePos.y/worldSpacePos.w << " " << ndcSpacePos.z/worldSpacePos.w << " " << worldSpacePos.w <<std::endl;
  return glm::vec3(worldSpacePos) / worldSpacePos.w;
}

void run_patch(Shader shader, Camera mycamera) {
  //Object
  std::vector<std::vector<BezierSurface>> objs ={
      {BezierSurface(test_surface_control_point), BezierSurface(test_surface_control_point_2)},
      {BezierSurface(test_surface_control_point_3), BezierSurface(test_surface_control_point_4)}
  };
  PatchSurface(objs);
  Object obj1 = objs[0][0].generateObject(31);
  obj1.setDrawMode(GL_TRIANGLES);
  Object obj2 = objs[0][1].generateObject(31);
  obj2.setDrawMode(GL_TRIANGLES);
  Object obj3 = objs[1][0].generateObject(31);
  obj3.setDrawMode(GL_TRIANGLES);
  Object obj4 = objs[1][1].generateObject(31);
  obj4.setDrawMode(GL_TRIANGLES);

  //user shader
  glm::mat4 view = mycamera.getViewMatrix();
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH/(float)HEIGHT, 0.1f, 100.0f);

  shader.use();
  shader.setVec3("lightColor",  vec3{1.0f, 1.0f, 1.0f});
  shader.setVec3("lightPos", lightPos);
  shader.setVec3("viewPos", mycamera.Position);

  shader.setMat4("view", view);
  shader.setMat4("projection", projection);
  shader.setMat4("model", glm::mat4(1.0f));

  shader.setFloat("ambientStrength", 0.2f);
  shader.setVec3("objectColor", vec3{1.0f, 0.5f, 0.31f});
  obj1.draw();

  shader.setVec3("objectColor", vec3{0.3f, 0.5f, 0.31f});
  obj2.draw();

  shader.setVec3("objectColor", vec3{0.3f, 0.5f, 0.31f});
  obj3.draw();

  shader.setVec3("objectColor", vec3{1.0f, 0.5f, 0.31f});
  obj4.draw();

  shader.setVec3("objectColor", vec3{1.0f, 1.0f, 1.0f});
  for (int i = 0; i < test_surface_control_point.size(); ++i) {
    for (int j = 0; j < test_surface_control_point[i].size(); ++j) {
      vec3 point = test_surface_control_point[i][j];
      Object test_point = generateSphere(point, 0.05, 20);
      test_point.setDrawMode(GL_TRIANGLES);
      test_point.draw();

      vec3 point_2 = test_surface_control_point_2[i][j];
      Object test_point_2 = generateSphere(point_2, 0.05, 20);
      test_point_2.setDrawMode(GL_TRIANGLES);
      test_point_2.draw();

      vec3 point_3 = test_surface_control_point_3[i][j];
      Object test_point_3 = generateSphere(point_3, 0.05, 20);
      test_point_3.setDrawMode(GL_TRIANGLES);
      test_point_3.draw();

      vec3 point_4 = test_surface_control_point_4[i][j];
      Object test_point_4 = generateSphere(point_4, 0.05, 20);
      test_point_4.setDrawMode(GL_TRIANGLES);
      test_point_4.draw();
    }
  }
}

void run_interaction(Shader shader, Camera mycamera) {
  //Object
  Object obj1 = BezierSurface(interaction_point).generateObject(40);
  obj1.setDrawMode(GL_TRIANGLES);
  
  //user shader
  glm::mat4 view = mycamera.getViewMatrix();
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH/(float)HEIGHT, 0.1f, 100.0f);

  shader.use();
  shader.setVec3("lightColor",  vec3{1.0f, 1.0f, 1.0f});
  shader.setVec3("lightPos", lightPos);
  shader.setVec3("viewPos", mycamera.Position);

  shader.setMat4("view", view);
  shader.setMat4("projection", projection);
  shader.setMat4("model", glm::mat4(1.0f));

  shader.setFloat("ambientStrength", 0.2f);
  shader.setVec3("objectColor", vec3{1.0f, 0.5f, 0.31f});
  obj1.draw();

  for (int i = 0; i < interaction_point.size(); ++i) {
    for (int j = 0; j < interaction_point[i].size(); ++j) {
      vec3 point = interaction_point[i][j];
      vec2 screen_position = WorldToScreen(point, projection, view, WIDTH, HEIGHT);
      float dist = distance(screen_position, vec2{lastX, lastY});
      if (dist < 20) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
          selected_point = {i, j};
          selected_flag = true;
        }
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
          selected_flag = false;
        }
        else {
          shader.setVec3("objectColor", vec3{1.0f, 0.0f, 0.0f});
        }
      }
      else {
        shader.setVec3("objectColor", vec3{1.0f, 1.0f, 1.0f});
      }
      Object test_point = generateSphere(point, 0.1, 40);
      test_point.setDrawMode(GL_TRIANGLES);
      test_point.draw();
    }
  }

  if (selected_flag) {
    vec3 mouse_world = ScreenToWorld(lastX, lastY, projection, view, WIDTH, HEIGHT);
    float ratio = (glm::length(mouse_world - mycamera.Position) / glm::length(interaction_point[selected_point.x][selected_point.y] - mycamera.Position));
    vec3 pos = (mouse_world - mycamera.Position) / ratio + mycamera.Position;
    interaction_point[selected_point.x][selected_point.y] = pos;
  }
}

void run_bspline(Shader shader, Camera mycamera) {
  //Object
  Object obj1 = BSplineSurface(interaction_point).generateObject(40);
  obj1.setDrawMode(GL_TRIANGLES);

  //user shader
  glm::mat4 view = mycamera.getViewMatrix();
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH/(float)HEIGHT, 0.1f, 100.0f);

  shader.use();
  shader.setVec3("lightColor",  vec3{1.0f, 1.0f, 1.0f});
  shader.setVec3("lightPos", lightPos);
  shader.setVec3("viewPos", mycamera.Position);

  shader.setMat4("view", view);
  shader.setMat4("projection", projection);
  shader.setMat4("model", glm::mat4(1.0f));

  shader.setFloat("ambientStrength", 0.2f);
  shader.setVec3("objectColor", vec3{1.0f, 0.5f, 0.31f});
  obj1.draw();

  for (int i = 0; i < interaction_point.size(); ++i) {
    for (int j = 0; j < interaction_point[i].size(); ++j) {
      vec3 point = interaction_point[i][j];
      vec2 screen_position = WorldToScreen(point, projection, view, WIDTH, HEIGHT);
      float dist = distance(screen_position, vec2{lastX, lastY});
      if (dist < 20) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
          selected_point = {i, j};
          selected_flag = true;
        }
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
          selected_flag = false;
        }
        else {
          shader.setVec3("objectColor", vec3{1.0f, 0.0f, 0.0f});
        }
      }
      else {
        shader.setVec3("objectColor", vec3{1.0f, 1.0f, 1.0f});
      }
      Object test_point = generateSphere(point, 0.1, 40);
      test_point.setDrawMode(GL_TRIANGLES);
      test_point.draw();
    }
  }

  if (selected_flag) {
    vec3 mouse_world = ScreenToWorld(lastX, lastY, projection, view, WIDTH, HEIGHT);
    float ratio = (glm::length(mouse_world - mycamera.Position) / glm::length(interaction_point[selected_point.x][selected_point.y] - mycamera.Position));
    vec3 pos = (mouse_world - mycamera.Position) / ratio + mycamera.Position;
    interaction_point[selected_point.x][selected_point.y] = pos;
  }
}
