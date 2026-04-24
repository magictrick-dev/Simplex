CMAKE_MINIMUM_REQUIRED(VERSION 3.21)

PROJECT(Simplex)

SET(CMAKE_CXX_COMPILER clang++)
SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
SET(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
SET(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
SET(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
ADD_SUBDIRECTORY("vendor/glfw")
ADD_SUBDIRECTORY("vendor/glm/GLM")

IF (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    ADD_COMPILE_OPTIONS(-Wno-nontrivial-memcall)
ENDIF (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")

IF (WIN32)
    FIND_PACKAGE(OpenGL REQUIRED)
    ADD_EXECUTABLE(Simplex WIN32
        "vendor/glad/GLAD/glad.c"
        "vendor/glad/GLAD/glad_wgl.c"
        "vendor/IMGUI/imgui/backends/imgui_impl_glfw.cpp"
        "vendor/IMGUI/imgui/backends/imgui_impl_opengl3.cpp"
        "vendor/IMGUI/imgui/imgui.cpp"
        "vendor/IMGUI/imgui/imgui_demo.cpp"
        "vendor/IMGUI/imgui/imgui_draw.cpp"
        "vendor/IMGUI/imgui/imgui_tables.cpp"
        "vendor/IMGUI/imgui/imgui_widgets.cpp"
        "vendor/IMGUI/imgui/implot.cpp"
        "vendor/IMGUI/imgui/implot.h"
        "vendor/IMGUI/imgui/implot_demo.cpp"
        "vendor/IMGUI/imgui/implot_internal.h"
        "vendor/IMGUI/imgui/implot_items.cpp"

        "source/cli.cpp"
        "source/entry.cpp"
    )

    SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Simplex)
ENDIF (WIN32)

IF (APPLE)
    FIND_PACKAGE(OpenGL REQUIRED)
    ADD_EXECUTABLE(Simplex
        "vendor/glad/GLAD/glad.c"
        "vendor/IMGUI/imgui/backends/imgui_impl_glfw.cpp"
        "vendor/IMGUI/imgui/backends/imgui_impl_opengl3.cpp"
        "vendor/IMGUI/imgui/imgui.cpp"
        "vendor/IMGUI/imgui/imgui_demo.cpp"
        "vendor/IMGUI/imgui/imgui_draw.cpp"
        "vendor/IMGUI/imgui/imgui_tables.cpp"
        "vendor/IMGUI/imgui/imgui_widgets.cpp"
        "vendor/IMGUI/imgui/implot.cpp"
        "vendor/IMGUI/imgui/implot.h"
        "vendor/IMGUI/imgui/implot_demo.cpp"
        "vendor/IMGUI/imgui/implot_internal.h"
        "vendor/IMGUI/imgui/implot_items.cpp"

        "source/cli.cpp"
        "source/entry.cpp"
    )
ENDIF (APPLE)

SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}../bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}../bin)
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET_PROPERTY(TARGET Simplex PROPERTY CXX_STANDARD 20)

TARGET_INCLUDE_DIRECTORIES(Simplex PUBLIC 
    "vendor/glad"
    "vendor/glm"
    "vendor/stb"
    "vendor/imgui"
    "vendor/glfw/include"
    "source"
    "include"
)

IF (WIN32)
    TARGET_LINK_LIBRARIES(Simplex OpenGL::GL glfw)
ENDIF (WIN32)

IF (APPLE)
    TARGET_LINK_LIBRARIES(Simplex OpenGL::GL glfw)
ENDIF(APPLE)
