# MyProject/Game/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(Game)

# Require C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files
set(GAME_SRC
    source/main.cpp
    source/firefly_scene.cpp
    source/firefly.cpp
)

# Create the executable
add_executable(Game ${GAME_SRC})

# Include directories for the game
target_include_directories(Game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Engine/include # Include the GameEngine headers
)

# Link the game with the game engine
target_link_libraries(Game PRIVATE GameEngine)
