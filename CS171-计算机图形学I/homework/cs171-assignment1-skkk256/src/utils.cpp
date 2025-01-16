#include <utils.h>

#ifdef USE_FILESYSTEM

  #include <filesystem>

std::string getPath(const std::string &target, int depth) {
  std::string path = target;
  namespace fs = std::filesystem;
  for (int i = 0; i < depth; ++i) {
    if (fs::exists(path)) { return path; }
    // Well... hopefully it works
    path = "../" + path;
  }
  LOG_ERR("failed to get file: " + target);
  return target;
}
#else

  #include <cstdio>

std::string getPath(const std::string &target, int depth) {
  std::string path = target;
  for (int i = 0; i < depth; ++i) {
    FILE *file = fopen(path.c_str(), "r");
    if (file != nullptr) {
      fclose(file);
      return path;
    }
    // Well... hopefully it works
    path = "../" + path;
  }
  LOG_ERR("failed to get file: " + target);
  return target;
}
#endif

WindowGuard::WindowGuard(GLFWwindow *&window, int width, int height,
                         const std::string &title) {
  if (glfwInit() == 0) { LOG_ERR("failed to init glfw"); }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  // Create window
  window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (window == nullptr) { LOG_ERR("failed to create glfw window"); }

  glfwMakeContextCurrent(window);

  // Enable vsync
  glfwSwapInterval(1);

  // Init glad
  // NOLINTNEXTLINE: retain the C-style cast for gladLoadGLLoader
  if (gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress)) == 0) {
    LOG_ERR("failed to init glad");
  }
  glViewport(0, 0, width, height);
}

WindowGuard::~WindowGuard() { glfwTerminate(); }
