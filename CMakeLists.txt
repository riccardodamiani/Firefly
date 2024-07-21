# MyProject/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(GameProject)

# Require C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Add subdirectories
add_subdirectory(Engine)
add_subdirectory(Game)
