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
add_library(ImageRotation include/imageRotation.h src/imageRotation.cpp)
add_library(Utils include/category.h include/minimapConstants.h include/util.h src/util_first.cpp src/util_second.cpp)

target_link_libraries(Ball
    ${OpenCV_LIBS}
    Utils
)

target_link_libraries(Table
    ${OpenCV_LIBS}
)

target_link_libraries(Detection
    ${OpenCV_LIBS}
    Ball
    Table
    Utils
)

target_link_libraries(Segmentation
    ${OpenCV_LIBS}
    Ball
    Table
    Detection
    Utils
)

target_link_libraries(TableOrientation
    ${OpenCV_LIBS}
    Ball
    Table
    Detection
    Utils
)

target_link_libraries(Transformation
    ${OpenCV_LIBS}
    Ball
    Table
    TableOrientation
    Utils
)

target_link_libraries(Metrics
    ${OpenCV_LIBS}
    Ball
    Table
    Utils
)

target_link_libraries(Tracking
    ${OpenCV_LIBS}
    Ball
    Table
    Metrics
    Utils
)

target_link_libraries(Utils
    ${OpenCV_LIBS}
)


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
    ImageRotation
    Utils
)

add_executable(ShowSegmentationColored src/showSegmentationColored.cpp)
target_link_libraries(ShowSegmentationColored
    ${OpenCV_LIBS}
    Ball
    Table
    Detection
    Segmentation
    Utils
    Metrics
)

add_executable(TestAllClip src/testAllClip.cpp)
target_link_libraries(TestAllClip
    ${OpenCV_LIBS}
    Ball
    Table
    Detection
    Segmentation
    Utils
    Metrics
)

add_executable(ComputePerformance src/computePerformance.cpp)
target_link_libraries(ComputePerformance
    ${OpenCV_LIBS}
    Ball
    Table
    Detection
    Segmentation
    Utils
    Metrics
)

add_executable(RotateBbox src/rotateBboxesFile.cpp)
target_link_libraries(RotateBbox
    ${OpenCV_LIBS}
    Utils
)

add_executable(ShowBB src/showBB.cpp)
target_link_libraries(ShowBB
    ${OpenCV_LIBS}
    Utils
)
