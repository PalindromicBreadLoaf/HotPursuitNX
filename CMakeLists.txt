cmake_minimum_required(VERSION 3.16)
project(nfs_recompiled LANGUAGES C CXX ASM)

# options
option(WITH_PEDANTIC_FPU "Compile pedantic FPU implementation" OFF)
option(WITH_MMX "Enable MMX support" ON)

# add compile definitions based on options
if(WITH_PEDANTIC_FPU)
    add_compile_definitions(WITH_PEDANTIC_FPU)
endif()
if(WITH_MMX)
    add_compile_definitions(WITH_MMX)
endif()

# Detect Windows target: covers native Windows (WIN32/MINGW), an explicit
# -DCMAKE_SYSTEM_NAME=Windows, and cross-compilation via a MinGW toolchain
# supplied directly with -DCMAKE_C_COMPILER=…/x86_64-w64-mingw32-gcc (in
# which case WIN32/MINGW may not yet be set if no toolchain file is used).
if(WIN32 OR MINGW OR CMAKE_SYSTEM_NAME STREQUAL "Windows"
        OR CMAKE_C_COMPILER MATCHES "mingw"
        OR CMAKE_CXX_COMPILER MATCHES "mingw")
    set(TARGET_WINDOWS TRUE)
else()
    set(TARGET_WINDOWS FALSE)
endif()

if(NINTENDO_SWITCH OR CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
    set(TARGET_SWITCH TRUE)
else()
    set(TARGET_SWITCH FALSE)
endif()

# When building for Windows we have a local copy of SDL3 under the
# `sdl3` subdirectory.  Point CMake at it so find_package() will
# locate the bundled config file automatically.
if(TARGET_WINDOWS)
    message(STATUS "Windows target detected – adding ${CMAKE_SOURCE_DIR}/sdl3/cmake to CMAKE_PREFIX_PATH for SDL3")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/sdl3/cmake")
    set(SDL3_DIR "${CMAKE_SOURCE_DIR}/sdl3/cmake" CACHE PATH "SDL3 CMake directory")
endif()

if(TARGET_SWITCH)
    foreach(required_file
            "${DEVKITPRO}/libnx/include/switch.h"
            "${DEVKITPRO}/tools/bin/elf2nro"
            "${DEVKITPRO}/tools/bin/nacptool")
        if(NOT EXISTS "${required_file}")
            message(FATAL_ERROR
                "Required Nintendo Switch build dependency not found: ${required_file}\n"
                "Install or update devkitA64, libnx, and switch-tools.")
        endif()
    endforeach()

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(NXVK_GL QUIET nxvk-gl)
    if(NOT NXVK_GL_FOUND)
        message(FATAL_ERROR
            "nxvk-gl was not found in the Switch portlibs prefix. Install "
            "nxvk configuring.")
    endif()

    find_package(Git REQUIRED)
    include(FetchContent)

    set(SDL_SHARED OFF CACHE BOOL "Build a shared SDL library" FORCE)
    set(SDL_STATIC ON CACHE BOOL "Build a static SDL library" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build the SDL test library" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "Build the SDL test programs" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "Build the SDL example programs" FORCE)
    set(SDL_INSTALL OFF CACHE BOOL "Generate SDL install rules" FORCE)
    set(SDL_UNINSTALL OFF CACHE BOOL "Generate the SDL uninstall target" FORCE)

    FetchContent_Declare(sdl3_switch
        GIT_REPOSITORY https://github.com/SnepOMatic/SDL3-Switch.git
        GIT_TAG 6110ab7f8f1c131d2801689c52f3918a9e44287e
        GIT_PROGRESS TRUE)
    FetchContent_MakeAvailable(sdl3_switch)

    add_library(nfs_nxvk_gl INTERFACE)
    target_include_directories(nfs_nxvk_gl INTERFACE ${NXVK_GL_INCLUDE_DIRS})
    target_compile_options(nfs_nxvk_gl INTERFACE ${NXVK_GL_CFLAGS_OTHER})
    target_link_libraries(nfs_nxvk_gl INTERFACE ${NXVK_GL_LDFLAGS})
    set(OPENGL_TARGET nfs_nxvk_gl)
else()
    find_package(SDL3 REQUIRED)
    find_package(OpenGL REQUIRED)
    set(OPENGL_TARGET OpenGL::GL)
endif()

if (MSVC)
    add_compile_options(/W4 /WX)
elseif(TARGET_SWITCH)
    add_compile_options(-Wall -Wextra -Wstrict-aliasing=2)
else()
    add_compile_options(-Wall -Wextra -Werror -Wstrict-aliasing=2)
endif()

# prefer modern imported targets if available
if(TARGET SDL3::SDL3)
    set(SDL3_LIB_TARGET SDL3::SDL3)
else()
    include_directories(${SDL3_INCLUDE_DIRS})
    set(SDL3_LIB_TARGET ${SDL3_LIBRARIES})
endif()

# Keep SDL's non-graphics dependencies and let nxvk-gl satisfy SDL's EGL calls.
if(TARGET_SWITCH AND TARGET SDL3::SDL3-static)
    get_target_property(SDL3_STATIC_TARGET SDL3::SDL3-static ALIASED_TARGET)
    if(NOT SDL3_STATIC_TARGET)
        set(SDL3_STATIC_TARGET SDL3::SDL3-static)
    endif()
    get_target_property(SDL3_SWITCH_LIBS ${SDL3_STATIC_TARGET} INTERFACE_LINK_LIBRARIES)
    list(FILTER SDL3_SWITCH_LIBS EXCLUDE REGEX
        "^\\$<LINK_ONLY:(EGL|glapi|drm_nouveau)>$")
    set_property(TARGET ${SDL3_STATIC_TARGET} PROPERTY
        INTERFACE_LINK_LIBRARIES "${SDL3_SWITCH_LIBS}")
endif()

# SDL3main provides the WinMain→main entry-point shim required on Windows.
# The modern config exposes it as a separate target; the legacy FindSDL3.cmake
# already folds it into SDL3_LIBRARIES, so we only add it on top for the
# imported-target path.
if(TARGET SDL3::SDL3main)
    set(SDL3_MAIN_TARGET SDL3::SDL3main)
else()
    set(SDL3_MAIN_TARGET "")
endif()

file(GLOB NFS2SE_DISASM_SOURCES
     "${CMAKE_SOURCE_DIR}/src/nfs2se/disassembly/*.cpp")
file(GLOB NFS2SE_DISASM_HEADERS
     "${CMAKE_SOURCE_DIR}/src/nfs2se/disassembly/*.h")

file(GLOB NFS3HP_DISASM_SOURCES
     "${CMAKE_SOURCE_DIR}/src/nfs3hp/disassembly/*.cpp")
file(GLOB NFS3HP_DISASM_HEADERS
     "${CMAKE_SOURCE_DIR}/src/nfs3hp/disassembly/*.h")

# ---------------------------------------------------------------------------
# core static library
# ---------------------------------------------------------------------------

file(GLOB LIB_CPP_SOURCES
     "${CMAKE_SOURCE_DIR}/src/lib/*.cpp"
     "${CMAKE_SOURCE_DIR}/src/lib/sdl-backend/*.cpp"
     "${CMAKE_SOURCE_DIR}/src/lib/winapi/*.cpp"
     "${CMAKE_SOURCE_DIR}/src/lib/winapi/ddraw/*.cpp"
     "${CMAKE_SOURCE_DIR}/src/lib/winapi/dinput/*.cpp"
     "${CMAKE_SOURCE_DIR}/src/lib/winapi/dsound/*.cpp")

# x87.asm provides 80-bit extended-precision FPU helpers and is only needed
# (and only supported) on Linux with WITH_PEDANTIC_FPU=ON.
if(WITH_PEDANTIC_FPU AND (TARGET_WINDOWS OR TARGET_SWITCH))
    message(FATAL_ERROR
        "WITH_PEDANTIC_FPU is not supported on this target: the x87 NASM "
        "helper relies on Linux-specific x86-64 ABI conventions.")
endif()

if(WITH_PEDANTIC_FPU AND NOT TARGET_WINDOWS AND NOT TARGET_SWITCH)
    set(X87_OBJ "${CMAKE_CURRENT_BINARY_DIR}/x87.o")
    add_custom_command(
        OUTPUT ${X87_OBJ}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND nasm -f elf64 -o ${X87_OBJ} ${CMAKE_SOURCE_DIR}/src/lib/x87.asm
        DEPENDS ${CMAKE_SOURCE_DIR}/src/lib/x87.asm
        COMMENT "Assembling x87.asm (elf64, pedantic FPU)"
        VERBATIM
    )
    list(APPEND LIB_CPP_SOURCES ${X87_OBJ})
endif()

add_library(nfs_core STATIC ${LIB_CPP_SOURCES})
target_include_directories(nfs_core PRIVATE
    ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(nfs_core PRIVATE ${SDL3_MAIN_TARGET} ${SDL3_LIB_TARGET} ${OPENGL_TARGET})

# helper function for adding the game executables
function(add_game_exe name)
    # script and directories
    set(game_src_dir "${CMAKE_SOURCE_DIR}/src/${name}")

    # collect sources
    file(GLOB GAME_SOURCES "${game_src_dir}/*.cpp")

    # pick the hardcoded disasm set for this game
    if(name STREQUAL "nfs2se")
        set(DISASM_SOURCES ${NFS2SE_DISASM_SOURCES})
        set(DISASM_HEADERS ${NFS2SE_DISASM_HEADERS})
    elseif(name STREQUAL "nfs3hp")
        set(DISASM_SOURCES ${NFS3HP_DISASM_SOURCES})
        set(DISASM_HEADERS ${NFS3HP_DISASM_HEADERS})
    else()
        set(DISASM_SOURCES "")
        set(DISASM_HEADERS "")
    endif()

    # adding the headers to the executable source list is what makes CMake
    # aware that the custom command must run before compilation starts.
    add_executable(${name} ${GAME_SOURCES} ${DISASM_SOURCES} ${DISASM_HEADERS})
    set_source_files_properties(${DISASM_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
    target_include_directories(${name} PRIVATE
        ${game_src_dir}
        ${CMAKE_SOURCE_DIR}/src/${name}/disassembly
        ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${name} PRIVATE ${SDL3_MAIN_TARGET} nfs_core ${SDL3_LIB_TARGET} ${OPENGL_TARGET})
    if(WITH_PEDANTIC_FPU)
        target_compile_definitions(${name} PRIVATE WITH_PEDANTIC_FPU)
    endif()
    if(WITH_MMX)
        target_compile_definitions(${name} PRIVATE WITH_MMX)
    endif()
    if(TARGET_SWITCH)
        target_sources(${name} PRIVATE
            "${CMAKE_SOURCE_DIR}/src/lib/switch/nxvk_compat.c")
        if(name STREQUAL "nfs2se")
            set(game_title "Need for Speed II SE")
            set(game_icon "${CMAKE_SOURCE_DIR}/assets/icons/nfs2se.jpg")
        else()
            set(game_title "Need for Speed III HP")
            set(game_icon "${CMAKE_SOURCE_DIR}/assets/icons/nfs3hp.jpg")
        endif()
        set(nacp_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.nacp")
        nx_generate_nacp(
            OUTPUT "${nacp_file}"
            NAME "${game_title}"
            AUTHOR "PalindromicBreadLoaf"
            VERSION "1.0.0")
        nx_create_nro(${name}
            NACP "${nacp_file}"
            ICON "${game_icon}")
    endif()
endfunction()

add_game_exe(nfs2se)
add_game_exe(nfs3hp)
