cmake_minimum_required(VERSION 3.28)
project(Milling_Simulator)

set(CMAKE_CXX_STANDARD 26)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always -Wall -g")

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts")
endif()

# Dear ImGui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/dep/imgui)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)
set(IMGUI_SRC
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/dep/inc)

# Link directories
link_directories(${CMAKE_SOURCE_DIR}/dep/lib)

set(APP_SRC
        src/interface/gui/GuiBase.cpp
        src/interface/gui/GuiBase.h
        src/app/AppContext.h
        src/interface/gui/Gui.cpp
        src/interface/gui/Gui.h
        src/opengl/framebuffer/FrameBufferManager.cpp
        src/opengl/framebuffer/FrameBufferManager.h
        src/opengl/shader/Shader.h
        src/opengl/shader/Shader.cpp
        src/interface/camera/BaseCamera.h
        src/interface/camera/CameraAnchor.cpp
        src/interface/camera/CameraAnchor.h
        src/interface/input/InputHandler.cpp
        src/interface/input/InputHandler.h
        src/app/Scene.cpp
        src/app/Scene.h
        src/app/entity/light/PointLight.h
        src/app/entity/point/Point.h
        src/app/gcode/GCodeParser.cpp
        src/app/gcode/GCodeParser.h
        src/opengl/texture/Texture.cpp
)

# Add executables
add_executable(Milling_Simulator
        # Dependencies
        ${IMGUI_SRC}
        ${CMAKE_SOURCE_DIR}/src/glad.c
        # Application
        ${CMAKE_SOURCE_DIR}/src/main.cpp
        ${APP_SRC}
)

add_subdirectory(${CMAKE_SOURCE_DIR}/dep/nativefiledialog-extended)

include_directories(dep/nativefiledialog-extended)

# Link libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")             # macOS
    target_link_libraries(Milling_Simulator
            nfd
            ${CMAKE_SOURCE_DIR}/dep/lib/libglfw.3.4.dylib
            ${CMAKE_SOURCE_DIR}/dep/lib/libGLEW.2.2.0.dylib
            "-framework OpenGL"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
            "-framework CoreFoundation"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")        # Windows
    find_package(OpenGL REQUIRED)
    include_directories( ${OPENGL_INCLUDE_DIRS})
    target_link_libraries(Milling_Simulator
            ${CMAKE_SOURCE_DIR}/dep/lib/glfw3.lib
            ${CMAKE_SOURCE_DIR}/dep/lib/glew32.lib
            ${OPENGL_LIBRARIES}
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")          # Linux
    target_link_libraries(Milling_Simulator PRIVATE
            -lglfw
            -lGLEW
            -lGL
            -lX11
            -lpthread
            -lXrandr
            -lXi
            -ldl
            nfd
    )
endif()