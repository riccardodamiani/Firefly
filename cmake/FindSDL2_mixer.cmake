# cmake/FindSDL2_mixer.cmake

find_path(SDL2_MIXER_INCLUDE_DIR SDL_mixer.h
    PATHS ${CMAKE_SOURCE_DIR}/third-party/SDL-mixer/include
)

find_library(SDL2_MIXER_LIBRARY
    NAMES SDL2_mixer
    PATHS ${CMAKE_SOURCE_DIR}/third-party/SDL-mixer/lib/x64
)

find_path(SDL2_MIXER_CONFIG_DIR SDL2_mixerConfig.cmake
    PATHS ${CMAKE_SOURCE_DIR}/third-party/SDL-mixer/cmake
)

if (SDL2_MIXER_CONFIG_DIR)
    set(SDL2_MIXER_DIR ${SDL2_MIXER_CONFIG_DIR})
    find_package(SDL2_mixer CONFIG REQUIRED)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_mixer DEFAULT_MSG SDL2_MIXER_LIBRARY SDL2_MIXER_INCLUDE_DIR)

if (SDL2_MIXER_FOUND)
    set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARY})
    set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR})
endif()
