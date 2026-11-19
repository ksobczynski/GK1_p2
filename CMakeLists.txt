cmake_minimum_required(VERSION 3.16)

project(p2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(p2
    src/main.cpp
    src/model/Vertex.h
    src/model/Triangle.h
    src/model/Mesh.h
    src/model/BezierSurface.h
    src/model/BezierSurface.cpp
    src/model/SurfaceLoader.h
    src/model/SurfaceLoader.cpp
    src/model/Scene.h
    src/model/Scene.cpp
    src/render/FrameBuffer.h
    src/render/Rasterizer.h
    src/render/Lighting.h
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/CanvasWidget.h
    src/ui/CanvasWidget.cpp
     src/model/LightCurve.h
    src/model/LightCurve.cpp
)

target_link_libraries(p2 PRIVATE Qt6::Widgets)

target_include_directories(p2 PRIVATE src)
