cmake_minimum_required(VERSION 3.5)
project(volrend VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS  "-w -O3")
set(NO_OMP 1)
add_definitions(-DNO_OMP)

# Flags for compiling
#if(NO_OMP STREQUAL 1)
#    message(STATUS "OpenMP disabled.")
#	add_definitions(-DNO_OMP)
#else()
#    message(STATUS "OpenMP enabled.")
#    set(NO_OMP 0)
#    remove_definitions(-DNO_OMP)
#endif()

# Find OpenMP
find_package(OpenMP)
if(NO_OMP STREQUAL 0 AND OpenMP_FOUND)
	message(STATUS "OpenMP Found.")
    include_directories(${OpenMP_CXX_INCLUDE_DIRS})
endif()

# Add include path
include_directories(
	src
	3rdLibs
	include
)

# Find source files
file(GLOB_RECURSE SOURCE_FILES 
    src/*.cpp
	include/*.hpp)
	

# Specific flags
if(WIN32)
	add_definitions(-D_AMD64_)
endif()

# Executables
add_executable(volrend ${SOURCE_FILES})
# Link OpenMP
if(NO_OMP STREQUAL 0)
	target_link_libraries(volrend OpenMP::OpenMP_CXX)
endif()

