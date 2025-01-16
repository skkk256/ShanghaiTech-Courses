# find_package(OpenMP REQUIRED)

# Include source files without main.cpp as lib
aux_source_directory("${PROJECT_SOURCE_DIR}/src" source)
list(REMOVE_ITEM source "${PROJECT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM source "${PROJECT_SOURCE_DIR}/src/conventions.cpp")

# # Add library
add_library(renderer_lib STATIC "${source}")
add_library(renderer::lib ALIAS renderer_lib)
set_property(TARGET renderer_lib PROPERTY CXX_STANDARD 17)

# Include libraries
target_link_libraries(renderer_lib PUBLIC 
  stb fmt nlohmann_json tinyobjloader
  linalg fmt spdlog tinyexr)
#target_link_libraries(renderer_lib PUBLIC
#        stb OpenMP::OpenMP_CXX fmt nlohmann_json tinyobjloader
#        linalg fmt spdlog tinyexr)
if (USE_EMBREE)
  target_link_libraries(renderer_lib PUBLIC embree)
  target_compile_definitions(renderer_lib PUBLIC USE_EMBREE)
  message(STATUS "Embree is enabled")
else()
  message(STATUS "Embree is disabled")
endif()
target_include_directories(renderer_lib PUBLIC "${PROJECT_SOURCE_DIR}/include")

add_executable(renderer "${PROJECT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(renderer PRIVATE renderer::lib)

add_executable(conventions "${PROJECT_SOURCE_DIR}/src/conventions.cpp")
target_link_libraries(conventions PRIVATE renderer::lib)

add_executable(exrtools "${PROJECT_SOURCE_DIR}/src/exrtools.cpp")
target_link_libraries(exrtools PRIVATE renderer::lib)
