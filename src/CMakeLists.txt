# CMakeLists - kernel/src
#
message(STATUS "Building framework")

set(CMAKE_STATIC_LIBRARY_PREFIX "sf${PROJECT_VERSION_MAJOR}_")

sus_add_all_framework_modules()

add_library(susfwk INTERFACE)
foreach(module ${SUSFWK_AVAILABLE_MODULES})
    target_link_libraries(susfwk INTERFACE
        $<TARGET_NAME_IF_EXISTS:${module}>
    )
endforeach()
target_include_directories(susfwk INTERFACE
    $<BUILD_INTERFACE:${SUSFWK_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(TARGETS susfwk
    EXPORT susfwkTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
