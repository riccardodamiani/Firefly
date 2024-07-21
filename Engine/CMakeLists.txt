# MyProject/GameEngine/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(FireflyEngine)

# Require C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find SDL2 and its extensions
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

# Add the source files for the game engine
add_library(FireflyEngine STATIC
    source/AnimatedSprite.cpp
    source/animation.cpp
    source/audio_source.cpp
    source/audio.cpp
    source/camera.cpp
    source/gameEngine.cpp
    source/gameObject.cpp
    source/graphics.cpp
    source/gui_button.cpp
    source/gui_droplist.cpp
    source/gui_text.cpp
    source/gui_editbox.cpp
    source/gui_slider.cpp
    source/gui_element.cpp
    source/gui_panel.cpp
    source/gui.cpp
    source/input.cpp
    source/lightObject.cpp
    source/multithreadManager.cpp
    source/physics.cpp
    source/platform.cpp
    source/rigidbody.cpp
    source/scene.cpp
    source/sprite.cpp
    source/threadHelper.cpp
    source/variables.cpp
)

# Include directories for SDL2
target_include_directories(FireflyEngine PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link SDL2 libraries
target_link_libraries(FireflyEngine PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
)
