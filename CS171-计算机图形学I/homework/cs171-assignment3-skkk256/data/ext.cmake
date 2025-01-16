# All packages required
CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:nlohmann/json@3.11.2")
CPMAddPackage("gh:tinyobjloader/tinyobjloader#v2.0.0rc10")
CPMAddPackage("gh:richgel999/miniz#3.0.2")

CPMAddPackage(
  NAME spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.11.0
  OPTIONS "SPDLOG_FMT_EXTERNAL ON"
)

CPMAddPackage(
  NAME tinyexr
  GIT_REPOSITORY https://github.com/syoyo/tinyexr.git
  GIT_TAG v1.0.2
  DOWNLOAD_ONLY YES
)

if (tinyexr_ADDED)
  add_library(tinyexr STATIC ${tinyexr_SOURCE_DIR}/tinyexr.cc)
  target_include_directories(tinyexr INTERFACE ${tinyexr_SOURCE_DIR})
  target_compile_definitions(tinyexr PUBLIC -DTINYEXR_USE_MINIZ=1 -DTINYEXR_USE_PIZ=1 
                                            -DTINYEXR_USE_OPENMP=0 -DTINYEXR_USE_STB_ZLIB=0)
  target_link_libraries(tinyexr PRIVATE miniz)
endif()

CPMAddPackage(
  NAME linalg
  GIT_REPOSITORY https://github.com/sgorsten/linalg.git
  GIT_TAG main
  DOWNLOAD_ONLY YES
)

if (linalg_ADDED)
  add_library(linalg INTERFACE)
  target_include_directories(linalg INTERFACE ${linalg_SOURCE_DIR})
endif()

CPMAddPackage(
  NAME stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
  DOWNLOAD_ONLY YES
)

if (stb_ADDED)
  add_library(stb INTERFACE)
  target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
  message(STATUS ${stb_SOURCE_DIR})
endif()

CPMAddPackage(
  NAME googletest
  GITHUB_REPOSITORY google/googletest
  GIT_TAG main
  OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
)

if (USE_EMBREE)
  # External BVH library
  CPMAddPackage(
    NAME embree
    GITHUB_REPOSITORY embree/embree
    GIT_TAG v4.1.0
    OPTIONS "EMBREE_ISPC_SUPPORT OFF" 
            "EMBREE_TUTORIALS OFF"
            "EMBREE_FILTER_FUNCTION OFF"
            "EMBREE_RAY_PACKETS OFF"
            "EMBREE_RAY_MASK OFF"
            "EMBREE_GEOMETRY_GRID OFF"
            "EMBREE_GEOMETRY_QUAD OFF"
            "EMBREE_GEOMETRY_CURVE OFF"
            "EMBREE_GEOMETRY_SUBDIVISION OFF"
            "EMBREE_GEOMETRY_USER OFF"
            "EMBREE_GEOMETRY_POINT OFF"
            "EMBREE_DISC_POINT_SELF_INTERSE OFF"
            
            "EMBREE_MAX_ISA NONE"
            "EMBREE_ISA_AVX OFF"
            "EMBREE_ISA_AVX2 ON"
            "EMBREE_ISA_AVX512 OFF"
            "EMBREE_ISA_SSE2 OFF"
            "EMBREE_ISA_SSE42 OFF"

            "EMBREE_TASKING_SYSTEM INTERNAL")
endif()