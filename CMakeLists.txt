CMAKE_MINIMUM_REQUIRED(VERSION 4.3)

PROJECT(Simplex)

FIND_PACKAGE(Python3 REQUIRED COMPONENTS Interpreter)
SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Place the compiled binary directly into the top-level build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Suppress the automatic /Debug or /Release subfolder addition for multi-config generators
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR})
endforeach()

IF (APPLE)
    ENABLE_LANGUAGE(OBJC)
    ENABLE_LANGUAGE(OBJCXX)
    SET(CMAKE_OBJCXX_STANDARD 23)
    SET(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
ENDIF (APPLE)

#SET(CMAKE_CXX_COMPILER clang++)
SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
SET(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
SET(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
SET(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
ADD_SUBDIRECTORY("extern/glfw")
ADD_SUBDIRECTORY("extern/glm")

IF (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    ADD_COMPILE_OPTIONS(-Wno-nontrivial-memcall)
ENDIF (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")

SET(PROJECT_SOURCES
    "source/entry.cpp"
    "source/utils/cli.cpp"
    "source/utils/linear/vec2.cpp"
    "source/utils/linear/vec3.cpp"
    "source/utils/linear/vec4.cpp"
    "source/utils/linear/mat4.cpp"
    "source/utils/system/memory_alloc.cpp"
    "source/utils/system/resource_manager.cpp"
    "source/parsers/rdview/rdview_token.cpp"
    "source/parsers/rdview/rdview_lexer.cpp"
    "source/parsers/rdview/rdview_node_types.cpp"
    "source/parsers/rdview/rdview_parser.cpp"

    "source/simplex/platform/glfw_window.cpp"

    "source/simplex/renderer/renderer_interface.cpp"
    "source/simplex/renderer/vulkan/vulkan_renderer.cpp"

    "extern/vendor/lz4/lz4.c"

    "source/scratch/scratch.cpp"
)

IF (WIN32)

    FIND_PACKAGE(OpenGL REQUIRED)
    FIND_PACKAGE(Vulkan REQUIRED)
    ADD_EXECUTABLE(Simplex WIN32
        "extern/vendor/glad/glad.c"
        "extern/vendor/imgui/backends/imgui_impl_glfw.cpp"
        "extern/vendor/imgui/backends/imgui_impl_opengl3.cpp"
        "extern/vendor/imgui/imgui.cpp"
        "extern/vendor/imgui/imgui_demo.cpp"
        "extern/vendor/imgui/imgui_draw.cpp"
        "extern/vendor/imgui/imgui_tables.cpp"
        "extern/vendor/imgui/imgui_widgets.cpp"
        "extern/vendor/imgui/implot.cpp"
        "extern/vendor/imgui/implot.h"
        "extern/vendor/imgui/implot_demo.cpp"
        "extern/vendor/imgui/implot_internal.h"
        "extern/vendor/imgui/implot_items.cpp"

        "source/simplex/platform/win32/win32_window.cpp"
        "source/simplex/platform/win32/win32_filesystem.cpp"

        ${PROJECT_SOURCES}
    )


    SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Simplex)
ENDIF (WIN32)

IF (APPLE)
    FIND_PACKAGE(Vulkan REQUIRED)
    FIND_PACKAGE(OpenGL REQUIRED)
    ADD_EXECUTABLE(Simplex
        "extern/vendor/glad/glad.c"
        "extern/vendor/imgui/backends/imgui_impl_glfw.cpp"
        "extern/vendor/imgui/backends/imgui_impl_opengl3.cpp"
        "extern/vendor/imgui/imgui.cpp"
        "extern/vendor/imgui/imgui_demo.cpp"
        "extern/vendor/imgui/imgui_draw.cpp"
        "extern/vendor/imgui/imgui_tables.cpp"
        "extern/vendor/imgui/imgui_widgets.cpp"
        "extern/vendor/imgui/implot.cpp"
        "extern/vendor/imgui/implot.h"
        "extern/vendor/imgui/implot_demo.cpp"
        "extern/vendor/imgui/implot_internal.h"
        "extern/vendor/imgui/implot_items.cpp"

        "source/simplex/platform/darwin/darwin_window.mm"
        "source/simplex/platform/darwin/darwin_filesystem.cpp"

        ${PROJECT_SOURCES}
    )
ENDIF (APPLE)

IF (LINUX)
    FIND_PACKAGE(Vulkan REQUIRED)
    FIND_PACKAGE(OpenGL REQUIRED)
    ADD_EXECUTABLE(Simplex
        "extern/vendor/glad/glad.c"
        "extern/vendor/imgui/backends/imgui_impl_glfw.cpp"
        "extern/vendor/imgui/backends/imgui_impl_opengl3.cpp"
        "extern/vendor/imgui/imgui.cpp"
        "extern/vendor/imgui/imgui_demo.cpp"
        "extern/vendor/imgui/imgui_draw.cpp"
        "extern/vendor/imgui/imgui_tables.cpp"
        "extern/vendor/imgui/imgui_widgets.cpp"
        "extern/vendor/imgui/implot.cpp"
        "extern/vendor/imgui/implot.h"
        "extern/vendor/imgui/implot_demo.cpp"
        "extern/vendor/imgui/implot_internal.h"
        "extern/vendor/imgui/implot_items.cpp"

        "source/simplex/platform/unix/x11_window.cpp"
        "source/simplex/platform/unix/unix_filesystem.cpp"

        ${PROJECT_SOURCES}
    )
ENDIF (LINUX)

TARGET_COMPILE_FEATURES(Simplex PRIVATE cxx_std_23)

SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}../bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}../bin)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

TARGET_INCLUDE_DIRECTORIES(Simplex PUBLIC 
    "extern/"
    "vendor/glfw/include"
    "source/"
    "include/"

    ${Vulkan_INCLUDE_DIRS}
)

IF (WIN32)
    TARGET_LINK_LIBRARIES(Simplex OpenGL::GL glfw shell32 ${Vulkan_LIBRARIES})
ENDIF (WIN32)

IF (APPLE)
    TARGET_LINK_LIBRARIES(Simplex OpenGL::GL glfw ${Vulkan_LIBRARIES}
        "-framework Cocoa" "-framework QuartzCore")
ENDIF(APPLE)

ADD_CUSTOM_COMMAND(TARGET Simplex POST_BUILD
    COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/scripts/bump_version.py
        ${CMAKE_SOURCE_DIR}/include/utils/about.hpp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Bumping patch version in about.hpp"
)