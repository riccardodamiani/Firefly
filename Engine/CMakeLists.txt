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

INCLUDE(CheckCXXSourceCompiles)

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


set(ATOMIC32_TEST_CODE "
    #include <atomic>
    #include <stdint.h>
    int main() {
        std::atomic<int32_t> x;
        x.store(1);
        x--;
        return x.load();
    }")

set(ATOMIC64_TEST_CODE "
    #include <atomic>
    #include <stdint.h>
    int main() {
        std::atomic<int64_t> x;
        x.store(1);
        x--;
        return x.load();
    }")

set(ATOMIC128_TEST_CODE "
    #include <atomic>
    #include <stdint.h>
    struct vector2{
        unsigned long long x;
        unsigned long long y;
    };
    int main() {
        std::atomic<vector2> x;
        x.store({1, 1});
        return x.load().x;
    }")

macro(ATOMIC_CHECK)
    # test whether atomic works
    check_cxx_source_compiles("${ATOMIC32_TEST_CODE}" atomic32_test)
    check_cxx_source_compiles("${ATOMIC64_TEST_CODE}" atomic64_test)
    check_cxx_source_compiles("${ATOMIC128_TEST_CODE}" atomic128_test)

    # if doesn't work, link atomic
    if(NOT atomic32_test OR NOT atomic64_test OR NOT atomic128_test)
        target_link_libraries(FireflyEngine PRIVATE atomic)
    endif()
endmacro()

ATOMIC_CHECK()

# Link SDL2 libraries
target_link_libraries(FireflyEngine PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
)
