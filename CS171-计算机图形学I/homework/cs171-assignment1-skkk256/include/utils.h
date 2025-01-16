// === ------------------------------------------------------------------------
//
// This file is part of ShanghaiTech CS171.01 Homework. It is \em NOT an open
// source project so students are strongly prohibited from redistributing it,
// i.e. posting it on any public platform, share code with other students.
// Redistributions to the project are considered a severe violation of
// plagiarism regulations.
//
// === -----------------------------------------------------------------------

#ifndef INCLUDE_UTILS_H_
#define INCLUDE_UTILS_H_

#include <string>

#include "defines.h"

/// Try and search target file in parent directories with a max depth limit
std::string getPath(const std::string &target, int depth = 5);

///
/// \brief WindowGuard is a RAII wrapper for GLFWwindow, which will release
/// the resource upon destruction, i.e., when it goes out of scope.
///
/// \see A list of readings on RAII
/// - https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization
/// - https://en.cppreference.com/w/cpp/language/raii
///
struct WindowGuard final {
  /// Construct the window with given width, height and title, and the
  /// corresponding GLFW *resource*.
  WindowGuard(GLFWwindow *&, int width, int height,
              const std::string &title);

  /// Destruct the window and release the resource (automatically).
  ~WindowGuard();
};

#endif  // INCLUDE_UTILS_H_
