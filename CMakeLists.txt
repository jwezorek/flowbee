cmake_minimum_required(VERSION 3.16)

project(flowbee VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.80 REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
endif()

find_package(Eigen3 3.3 REQUIRED)

add_executable(flowbee
    src/third-party/mixbox.cpp
    src/main.cpp
    src/util.cpp
    src/paint.cpp 
    src/brush.cpp
)

target_link_libraries(flowbee PRIVATE Eigen3::Eigen)

