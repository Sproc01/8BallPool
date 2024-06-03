# CMakeLists.txt
cmake_minimum_required(VERSION "2.8")

project(8BallPool)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} include/)

add_library(ObjectDetection include/objectDetection.h src/objectDetection.cpp)
add_library(Segment include/segment.h src/segment.cpp)
add_library(Table include/table.h src/table.cpp)
add_library(Ball include/ball.h src/ball.cpp)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME}
        ${OpenCV_LIBS}
        Table
        Ball
        ObjectDetection
        Segment)
