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



# add subdirectory with path to folder containing Scene-Serializer CMakeList.txt, this should include it in build process
add_subdirectory(dep/Scene-Serializer/cpp/Serializer)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/dep/inc)

# Link directories
link_directories(${CMAKE_SOURCE_DIR}/dep/lib)

set(APP_SRC
        dep/glad/glad.c
        dep/glad/glad_glfw.h
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
        src/app/entity/mill/MillModel.cpp
        src/app/entity/mill/MillModel.h
        src/app/entity/sphere/Sphere.cpp
        src/app/entity/cylinder/Cylinder.cpp
        src/app/algorithm/Bresenham.cpp
        src/app/algorithm/Bresenham.h
        src/app/mill/Mill.cpp
        src/app/mill/Mill.h
        src/app/mill/MillType.h
        src/app/algorithm/Bresenham3D.cpp
        src/app/algorithm/Bresenham3D.h
        src/app/heightmap/HeightMap.cpp
        src/app/heightmap/HeightMap.h
        src/app/model/serializer/Serializer.cpp
        src/app/model/serializer/Serializer.h
        src/app/entity/patchC0/PatchC0.h
        src/app/entity/patchC2/PatchC2.h
        src/app/entity/patchC2/PatchC2.h
        src/app/intersection/SurfaceIntersection.h
        src/app/intersection/ParametricSurface.h
        src/app/path/PathGenerator.cpp
        src/app/path/PathGenerator.h
        src/interface/camera/TopDownCamera.h
        src/app/gcode/GCodeExporter.cpp
        src/app/gcode/GCodeExporter.h
        src/app/intersectionMask/IntersectionMask.cpp
        src/app/intersectionMask/IntersectionMask.h
        src/app/io/stb_image.h
)

# Add executables
add_executable(Milling_Simulator
        # Dependencies
        ${IMGUI_SRC}
        # Application
        ${CMAKE_SOURCE_DIR}/src/main.cpp
        ${APP_SRC}
)

add_subdirectory(${CMAKE_SOURCE_DIR}/dep/nativefiledialog-extended)
add_subdirectory(dep/glfw)

include_directories(dep/nativefiledialog-extended dep/Scene-Serializer/cpp/Serializer/Serializer)

# Link libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")             # macOS
    target_link_libraries(Milling_Simulator
            MGSerializer
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
    find_package(glm CONFIG REQUIRED)
    find_package(OpenGL REQUIRED)
    include_directories( ${OPENGL_INCLUDE_DIRS})
    target_link_libraries(Milling_Simulator PRIVATE
            MGSerializer
            glm::glm
            nfd
            glfw
            ${OPENGL_LIBRARIES}
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")          # Linux
    target_link_libraries(Milling_Simulator PRIVATE
            MGSerializer
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