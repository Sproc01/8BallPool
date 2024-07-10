# CMakeLists.txt
cmake_minimum_required(VERSION "2.8")

project(8BallPool)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} include/)

add_library(Ball include/ball.h src/ball.cpp)
add_library(Table include/table.h src/table.cpp)
add_library(Detection include/detection.h src/detection.cpp)
add_library(Segmentation include/segmentation.h src/segmentation.cpp)
add_library(TableOrientation include/tableOrientation.h src/tableOrientation.cpp)
add_library(Transformation include/transformation.h src/transformation.cpp)
add_library(Tracking include/tracking.h src/tracking.cpp)
add_library(Metrics include/metrics.h src/metrics.cpp)
add_library(Utils include/category.h include/minimapConstants.h include/util.h src/util.cpp)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    Ball
    Table
    Detection
    Segmentation
    TableOrientation
    Transformation
    Tracking
    Metrics
    Utils
)

add_executable(InteractiveTracking src/testTrackingInteractive.cpp)
target_link_libraries(InteractiveTracking
    ${OpenCV_LIBS}
)

add_executable(ShowSegmentationColored src/showSegmentationColored.cpp)
target_link_libraries(ShowSegmentationColored
    ${OpenCV_LIBS}
    Utils
)

add_executable(TestAllClip src/TestAllClip.cpp)
target_link_libraries(TestAllClip
    ${OpenCV_LIBS}
    Ball
    Table
    Detection
    Segmentation
    Utils
    Metrics
)
