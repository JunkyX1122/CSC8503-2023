CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
project(server)

add_executable(server Main_Server.cpp)

target_include_directories(server PUBLIC .)

target_link_libraries(server LINK_PUBLIC NCLCoreClasses)
target_link_libraries(client LINK_PUBLIC CSC8503CoreClasses)
target_link_libraries(client LINK_PUBLIC OpenGLRendering)
target_link_libraries(client LINK_PUBLIC CSC8503)

target_precompile_headers(${PROJECT_NAME} PRIVATE
        <vector>
        <map>
        <stack>
        <list>
        <set>
        <string>
        <thread>
        <atomic>
        <functional>
        <iostream>
        <chrono>
        <sstream>

        "../NCLCoreClasses/Vector2i.h"
        "../NCLCoreClasses/Vector3i.h"
        "../NCLCoreClasses/Vector4i.h"

        "../NCLCoreClasses/Vector2.h"
        "../NCLCoreClasses/Vector3.h"
        "../NCLCoreClasses/Vector4.h"
        "../NCLCoreClasses/Quaternion.h"
        "../NCLCoreClasses/Plane.h"
        "../NCLCoreClasses/Matrix2.h"
        "../NCLCoreClasses/Matrix3.h"
        "../NCLCoreClasses/Matrix4.h"

        "../NCLCoreClasses/GameTimer.h"
)