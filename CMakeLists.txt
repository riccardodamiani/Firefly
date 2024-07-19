cmake_minimum_required(VERSION 3.22)
project(FireflyEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.26.3
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(SDL2)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# sdl2_image
FetchContent_Declare(
        SDL2_IMAGE
        GIT_REPOSITORY  https://github.com/libsdl-org/SDL_image
        GIT_TAG         97405e74e952f51b16c315ed5715b6b9de5a8a50
        GIT_PROGRESS    TRUE
)
#set(SDL2IMAGE_INSTALL OFF)
FetchContent_MakeAvailable(SDL2_IMAGE)

# sdl2 mixer
FetchContent_Declare(
        SDL2_MIXER
        GIT_REPOSITORY  https://github.com/libsdl-org/SDL_mixer
        GIT_TAG         7baa66f30930a21d8d24583fcc390dd354407f80
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(SDL2_MIXER)

# sdl2 ttf
FetchContent_Declare(
        SDL2_TTF
        GIT_REPOSITORY  https://github.com/libsdl-org/SDL_ttf
        GIT_TAG         4554e23b0f8c8d97311f109d6baa1fbadb0b3bc5
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(SDL2_TTF)

# Create the static library
add_library(FireflyEngine STATIC
  source/AnimatedSprite.cpp
  source/animation.cpp
  source/audio_source.cpp
  source/audio.cpp
  source/camera.cpp
  source/entity.cpp
  source/gameEngine.cpp
  source/gameObject.cpp
  source/graphics.cpp
  source/gui_button.cpp
  source/gui_droplist.cpp
  source/gui_editbox.cpp
  source/gui_element.cpp
  source/gui_panel.cpp
  source/gui_slider.cpp
  source/gui_text.cpp
  source/gui.cpp
  source/input.cpp
  source/lightObject.cpp
  source/multithreadManager.cpp
  source/physics.cpp
  source/platform.cpp
  source/rigidBody.cpp
  source/scene.cpp
  source/sprite.cpp
  source/threadHelper.cpp
  source/variables.cpp
)
target_include_directories(FireflyEngine PUBLIC
  ${SDL2_SOURCE_DIR}/include
  ${SDL2_mixer_SOURCE_DIR}/include
  ${SDL2_image_SOURCE_DIR}/include
  ${SDL2_ttf_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(FireflyEngine PRIVATE
  SDL2-static
  SDL2_mixer
  SDL2_image
  SDL2_ttf
)
