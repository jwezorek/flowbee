cmake_minimum_required(VERSION 3.16)

project(flowbee VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.80 REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
endif()

add_executable(flowbee
    src/third-party/mixbox.cpp
    src/main.cpp
    src/util.cpp
    src/paint_particle.cpp 
    src/brush.cpp
    src/canvas.cpp
    src/pigment.cpp
    src/vector_field.cpp
    src/flowbee.cpp
)

set_target_properties(${BUILD_TARGET} PROPERTIES LINK_FLAGS "/PROFILE")
