# cmake/ProjectConfig.cmake
#

# ------------------------------------------------------------------------------

find_package(Git REQUIRED)

execute_process(
    COMMAND "${GIT_EXECUTABLE}" config --get remote.origin.url
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE HOMEPAGE_URL
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND "${GIT_EXECUTABLE}" describe --tags --dirty --always
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(GIT_VERSION MATCHES "v?([0-9]+)\\.([0-9]+)\\.([0-9]+)")
    set(VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
else()
    set(VERSION "0.0.0")
endif()
set(SUSFWK_PROJECT_OPTIONS
    VERSION ${VERSION}
    DESCRIPTION "A comprehensive C++ ecosystem for building high-performance applications across multiple platforms"
    HOMEPAGE_URL "${HOMEPAGE_URL}"
    LANGUAGES CXX ASM
)
set(SUSFWK_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SUSFWK_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(SUSFWK_BINARY_DIR "${CMAKE_SOURCE_DIR}/build")
set(SUSFWK_MODULE_DIR "${CMAKE_SOURCE_DIR}/cmake")

# ------------------------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Release, Debug, RelWithDebInfo, MinSizeRel)" FORCE)
endif()
set(SUSFWK_CPU_DISPATCH "runtime" CACHE STRING "SIMD instruction set level")
set_property(CACHE SUSFWK_CPU_DISPATCH PROPERTY STRINGS runtime scalar sse41 avx2 avx512 neon sve2 sme2 rv64imafdcp rv64gc zve64x)

# ------------------------------------------------------------------------------
