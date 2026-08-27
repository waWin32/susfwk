# cmake/susfwkModules.cmake
#

macro(sus_define_modules name)
    string(TOUPPER ${name} name_upper)
    set(SUSFWK_${name_upper}_ALL_MODULES
        CACHE INTERNAL "All modules of the susfwk framework"
    )
    set(SUSFWK_${name_upper}_AVAILABLE_MODULES
        CACHE STRING "Available susfwk modules"
    )
    if(NOT SUSFWK_${name_upper}_AVAILABLE_MODULES)
        set(SUSFWK_${name_upper}_AVAILABLE_MODULES 
            ${SUSFWK_${name_upper}_ALL_MODULES} 
            CACHE STRING "Available susfwk modules" FORCE
        )
    endif()
    function(sus_add_all_${name}_modules)
        foreach(module ${SUSFWK_${name_upper}_AVAILABLE_MODULES})
            if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${module})
                add_subdirectory(${module})
                message(STATUS "  Module: ${module} added to ${name}")
            else()
                message(WARNING "  Module '${module}' not found in ${name}, skipping")
            endif()
        endforeach()
    endfunction()
endmacro()

