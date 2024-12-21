cmake_minimum_required(VERSION 3.15)
project(PostureChecker)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Source files
add_executable(PostureChecker 
    src/main.cpp
)

# Link libraries
target_link_libraries(PostureChecker PRIVATE
    ${OpenCV_LIBS}
) 