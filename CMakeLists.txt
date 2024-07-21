# MyProject/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(GameProject)

# Require C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32)
    set(SDL2_PATH "${CMAKE_SOURCE_DIR}/third-party/SDL/")
    set(SDL2_IMAGE_PATH "${CMAKE_SOURCE_DIR}/third-party/SDL-image/")
    set(SDL2_MIXER_PATH "${CMAKE_SOURCE_DIR}/third-party/SDL-mixer/")
    set(SDL2_TTF_PATH "${CMAKE_SOURCE_DIR}/third-party/SDL-ttf/")
endif (WIN32)


# Set the CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Add subdirectories
add_subdirectory(Engine)
add_subdirectory(Game)
