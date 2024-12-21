cmake_minimum_required(VERSION 3.10)
project(PostureAnalyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(posture_analyzer src/main.cpp src/posture_analyzer.cpp)

target_link_libraries(posture_analyzer PRIVATE
    ${OpenCV_LIBS}
)

target_include_directories(posture_analyzer PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)