####################
# A simple CMake Project File for your reference
# Modify whatever you want
# You can also use other project frameworks
####################

cmake_minimum_required(VERSION 3.18)

project(mass-spring-cloth-simulation)
set (CMAKE_CXX_STANDARD 20)

# ######### External liberaries #############
# glfw
option(GLFW_BUILD_TESTS off)
option(GLFW_BUILD_EXAMPLES off)
option(GLFW_BUILD_DOCS off)
add_subdirectory(extern/glfw)
include_directories(extern/glfw/include)
list(APPEND DEPENDENCIES glfw ${glfw_LIBRARIES})

# glad
set(GLAD_INCLUDE extern/glad/include)
set(GLAD_SRC extern/glad/src/glad.c
        src/mesh.cpp
        include/mesh.h
        src/utils.cpp)
include_directories(${GLAD_INCLUDE})

# glm
add_subdirectory(extern/glm)
list(APPEND DEPENDENCIES glm::glm)
# ############################################

# Add the main library
add_library(libmain
    src/camera.cpp
    src/cloth.cpp
    src/cloth_renderer.cpp
    src/cloth_simulator.cpp
    src/shader.cpp
    ${GLAD_SRC}
)
target_include_directories(libmain
    PUBLIC include
)
target_link_libraries(libmain PUBLIC ${DEPENDENCIES})

# The main opengl framework for simulation and rendering
add_executable(main test/main.cpp)
target_link_libraries(main PUBLIC libmain)