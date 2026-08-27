# CMakeLists - kernel
#
cmake_minimum_required(VERSION 3.16)
include(cmake/ProjectConfig.cmake)
project(susfwk ${SUSFWK_PROJECT_OPTIONS})
include(cmake/susfwkHelpers.cmake)
include(cmake/susfwkModules.cmake)
include(CTest)
sus_init()
message(STATUS "")
message(STATUS "═══════════════════════════════════════════════════════════════")
message(STATUS "  ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "  ${PROJECT_DESCRIPTION}")
message(STATUS "  ${PROJECT_HOMEPAGE_URL}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build system: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Build arch: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  Cpu dispatch: ${SUSFWK_CPU_DISPATCH}")
message(STATUS "═══════════════════════════════════════════════════════════════")
message(STATUS "")

# ------------------------------------------------------------------------------

sus_define_modules(framework)

add_subdirectory(src)
option(BUILD_BENCHMARKS "Build benchmark tests" OFF)
option(BUILD_EXAMPLES "Build example applications" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
if(BUILD_TESTING OR BUILD_BENCHMARKS)
    enable_testing()
endif()
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# ------------------------------------------------------------------------------

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/susfwk
)
install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    OPTIONAL
)
include(CMakePackageConfigHelpers)
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/susfwk")
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/susfwkConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/susfwkConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/susfwkConfig.cmake"
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/susfwkConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/susfwkConfigVersion.cmake"
    DESTINATION ${CONFIG_INSTALL_DIR}
)
install(EXPORT susfwkTargets
    NAMESPACE susfwk::
    DESTINATION ${CONFIG_INSTALL_DIR}
)

# ------------------------------------------------------------------------------
