# CMakeLists.txt
cmake_minimum_required(VERSION "2.8")

project(8BallPool)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} include/)

add_library(ObjectDetection src/ObjectDetection.cpp)

include_directories(include)

add_library(Table src/table.cpp include/table.h)
add_library(Ball src/ball.cpp include/ball.h)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME}
        ${OpenCV_LIBS}
        Table
        ObjectDetection)
