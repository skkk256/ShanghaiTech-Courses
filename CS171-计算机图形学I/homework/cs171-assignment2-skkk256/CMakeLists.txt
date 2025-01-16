cmake_minimum_required(VERSION 3.16)
project(CS171-hw2 LANGUAGES C CXX)

option(USE_FILESYSTEM "use std::filesystem" off)

# glfw
option(GLFW_BUILD_TESTS off)
option(GLFW_BUILD_EXAMPLES off)
option(GLFW_BUILD_DOCS off)
add_subdirectory(libs/glfw)
include_directories(libs/glfw/include)
list(APPEND LIBRARIES glfw ${glfw_LIBRARIES})

# glew
option(BUILD_UTILS off)
add_subdirectory(libs/glew/build/cmake)
include_directories(libs/glew/include)
list(APPEND LIBRARIES glew_s)

# glm
add_subdirectory(libs/glm)
list(APPEND LIBRARIES glm::glm)

# add filesystem
if (USE_FILESYSTEM)
  list(APPEND DEFINES -DUSE_FILESYSTEM)
  if (NOT MSVC)
    list(APPEND LIBRARIES stdc++fs)
  endif()
endif()

# set source files and add executable
file(GLOB SRC_FILES src/*.cpp)

add_executable(main ${SRC_FILES} ${INTERNAL_SRC_FILES}
        include/data.h
        include/bspline.h
        src/bspline.cpp)
target_include_directories(main PRIVATE include)
target_compile_features(main PRIVATE cxx_std_17)
target_compile_definitions(main PRIVATE ${DEFINES})
target_link_libraries(main PRIVATE ${LIBRARIES})