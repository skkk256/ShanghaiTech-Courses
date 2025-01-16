cmake_minimum_required( VERSION 2.8 )
project( directMethod )

set( CMAKE_BUILD_TYPE Debug )
set( CMAKE_CXX_FLAGS "-std=c++11 -O3" )

# 添加cmake模块路径
list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules )

find_package( OpenCV )
include_directories( ${OpenCV_INCLUDE_DIRS} )
if(OpenCV_FOUND)
    # 打印 OpenCV 的包含目录
    message(STATUS "OpenCV include directory: ${OpenCV_INCLUDE_DIRS}")

    include_directories(${OpenCV_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "OpenCV not found. Please specify the OpenCV directory using OpenCV_DIR variable")
endif()


find_package( G2O )
include_directories( ${G2O_INCLUDE_DIRS} ) 

include_directories( "/usr/include/eigen3" )
# include_directories( "/usr/include/opencv4" )


set( G2O_LIBS 
    g2o_core g2o_types_sba g2o_solver_csparse g2o_stuff g2o_csparse_extension 
)

add_executable( direct_sparse direct_sparse.cpp )
target_link_libraries( direct_sparse ${OpenCV_LIBS} ${G2O_LIBS} )

add_executable( direct_dense direct_dense.cpp )
target_link_libraries( direct_dense ${OpenCV_LIBS} ${G2O_LIBS} )

# add_executable( direct_method direct_method.cpp )
# target_link_libraries( direct_method ${OpenCV_LIBS} ${G2O_LIBS} )