# cmake/susfwkHelpers.cmake
#

# Initializing a project with the susfwk framework
macro(sus_init)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.0)
        message(FATAL_ERROR 
            "  susfwk requires GNU Compiler 11.0 or higher\n"
            "  Found: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}\n"
        )
    endif()
    include(GNUInstallDirs)
endmacro()

# Set the basic settings for the general purpose.
function(sus_base_target_setup name)
    set_target_properties(${name} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
    target_compile_options(${name} PRIVATE
        -pipe
        -Wall
        -fno-exceptions
        -fno-rtti
        -fno-stack-protector
        -fno-stack-check
        -ffunction-sections
        -fdata-sections
        -fno-builtin
        -fno-ident
        -fno-use-cxa-atexit
        -fno-threadsafe-statics
        -fno-common
        $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp -masm=intel>
        $<$<CONFIG:Debug>:-g3 -O0 -DDEBUG>
        $<$<CONFIG:RelWithDebInfo>:-g -Og -DRELEASE>
        $<$<CONFIG:MinSizeRel>:-Os -DRELEASE -fno-asynchronous-unwind-tables>
        $<$<CONFIG:Release>:-finline-functions -DRELEASE -fno-asynchronous-unwind-tables -fmerge-all-constants -fno-semantic-interposition -Ofast -flto=auto>
    )
    target_link_options(${name} PRIVATE -Wl,-nostdlib -nodefaultlibs -nostartfiles -nostdlib++)
    if(WIN32)
        target_compile_definitions(${name} PRIVATE SYSTEM_NAME_WINDOWS)
    elseif(APPLE)
        target_compile_definitions(${name} PRIVATE SYSTEM_NAME_APPLE)
    elseif(UNIX)
        target_compile_definitions(${name} PRIVATE SYSTEM_NAME_LINUX)
    else()
        target_compile_definitions(${name} PRIVATE SYSTEM_NAME_BARE)
    endif()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
        target_compile_definitions(${name} PRIVATE SYSTEM_ARCH_X86_64)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
        target_compile_definitions(${name} PRIVATE SYSTEM_ARCH_ARM64)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv64|riscv64gc")
        target_compile_definitions(${name} PRIVATE SYSTEM_ARCH_RISCV64)
    else()
        target_compile_definitions(${name} PRIVATE SYSTEM_ARCH_UNKNOWN)
    endif()
    string(TOUPPER "${SUSFWK_CPU_DISPATCH}" SUSFWK_CPU_DISPATCH_UPPER)
    target_compile_definitions(${name} PRIVATE TARGET_FEATURE_${SUSFWK_CPU_DISPATCH_UPPER})
    target_compile_options(${name} PRIVATE
        $<$<STREQUAL:${SUSFWK_CPU_DISPATCH},sse41>:-msse4.1>
        $<$<STREQUAL:${SUSFWK_CPU_DISPATCH},avx2>:-mavx2>
        $<$<STREQUAL:${SUSFWK_CPU_DISPATCH},avx512>:-mavx512f>
        $<$<STREQUAL:${SUSFWK_CPU_DISPATCH},sve2>:-march=armv9-a+sve2>
        $<$<STREQUAL:${SUSFWK_CPU_DISPATCH},sme2>:-march=armv9.2-a+sme2>
        $<$<STREQUAL:${SUSFWK_CPU_DISPATCH},rv64imafdcp>:-march=rv64imafdcp>
        $<$<STREQUAL:${SUSFWK_CPU_DISPATCH},rv64gc>:-march=rv64gc>
        $<$<STREQUAL:${SUSFWK_CPU_DISPATCH},zve64x>:-march=rv64gcv_zve64xc>
    )
endfunction()

# Create an executable program under susfwk.
macro(sus_add_executable name)
    add_executable(${name} ${ARGN})
    sus_base_target_setup(${name})
    if(WIN32)
         target_link_options(${name} PRIVATE
            -Wl,--gc-sections
            -Wl,--no-undefined
            -Wl,--entry,_start
            -Wl,--subsystem,console
        )
    elseif(APPLE)
        target_link_options(${name} PRIVATE
            -Wl,-no_implicit_dylibs
            -Wl,-dead_strip
            -Wl,-no_undefined
            -Wl,-e,_start
        )
    elseif(UNIX)
        target_link_options(${name} PRIVATE
            -Wl,--gc-sections
            -Wl,--no-undefined
            -Wl,-e,_start
        )
    endif()
    target_link_options(${name} PRIVATE $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-Wl,--strip-all>)
endmacro()

# Create a static library for susfwk.
macro(sus_add_library name)
    add_library(${name} ${ARGN})
    sus_base_target_setup(${name})
    if(WIN32)
        target_link_options(${name} PRIVATE
            -Wl,--gc-sections
            -Wl,--no-undefined
        )
    elseif(APPLE)
        target_link_options(${name} PRIVATE
            -Wl,-dead_strip
            -Wl,-no_undefined
        )
    elseif(UNIX)
        target_link_options(${name} PRIVATE
            -Wl,--gc-sections
            -Wl,--no-undefined
        )
    endif()
endmacro()

# Create a dynamically linked library for susfwk.
macro(sus_add_module name)
    add_library(${name} ${ARGN})
    sus_base_target_setup(${name})
    if(WIN32)
        target_link_options(${name} PRIVATE
            -Wl,--gc-sections
            -Wl,--no-undefined
            -Wl,--dll
        )
    elseif(APPLE)
        target_link_options(${name} PRIVATE
            -Wl,-dead_strip
            -Wl,-no_undefined
            -Wl,-no_implicit_dylibs
        )
    elseif(UNIX)
        target_link_options(${name} PRIVATE
            -Wl,--gc-sections
            -Wl,--no-undefined
        )
    endif()
    target_link_options(${name} PRIVATE $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-Wl,--strip-all>)
endmacro()
