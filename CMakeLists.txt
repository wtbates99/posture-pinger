cmake_minimum_required(VERSION 3.15)
project(PostureChecker)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings
if(WIN32)
    # Add Windows-specific include directories
    include_directories("$ENV{WINDOWSSDKDIR}Include")
endif()

# Find required packages
find_package(OpenCV REQUIRED)
find_package(SQLite3 REQUIRED)

# Add nlohmann_json
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Source files
set(SOURCES
    src/main.cpp
    src/PostureChecker.cpp
    src/PoseDetector.cpp
    src/Statistics.cpp
    src/Settings.cpp
)

# Add resource.rc only on Windows
if(WIN32)
    list(APPEND SOURCES src/resource.rc)
endif()

# Headers
set(HEADERS
    src/PostureChecker.hpp
    src/PoseDetector.hpp
    src/Statistics.hpp
    src/Settings.hpp
)

if(WIN32)
    list(APPEND HEADERS src/resource.h)
endif()

# Create executable
if(WIN32)
    add_executable(PostureChecker WIN32 ${SOURCES} ${HEADERS})
else()
    add_executable(PostureChecker ${SOURCES} ${HEADERS})
endif()

# Include directories
target_include_directories(PostureChecker PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(PostureChecker PRIVATE
    ${OpenCV_LIBS}
    SQLite::SQLite3
    nlohmann_json::nlohmann_json
)

# Windows-specific settings
if(WIN32)
    target_link_libraries(PostureChecker PRIVATE
        comctl32
        gdi32
        shell32
    )
endif()

# Copy assets to build directory
add_custom_command(TARGET PostureChecker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:PostureChecker>/assets
)

# Install targets
install(TARGETS PostureChecker
    RUNTIME DESTINATION bin
)

install(DIRECTORY assets/
    DESTINATION bin/assets
) 