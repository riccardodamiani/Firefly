# cmake/FindSDL2_ttf.cmake

find_path(SDL2_TTF_INCLUDE_DIR SDL_ttf.h
    PATHS ${CMAKE_SOURCE_DIR}/third-party/SDL-ttf/include
)

find_library(SDL2_TTF_LIBRARY
    NAMES SDL2_ttf
    PATHS ${CMAKE_SOURCE_DIR}/third-party/SDL-ttf/lib/x64
)

find_path(SDL2_TTF_CONFIG_DIR sdl2_ttfconfig.cmake
    PATHS ${CMAKE_SOURCE_DIR}/third-party/SDL-ttf/cmake
)

if (SDL2_TTF_CONFIG_DIR)
    set(SDL2_TTF_DIR ${SDL2_TTF_CONFIG_DIR})
    find_package(SDL2_ttf CONFIG REQUIRED)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_ttf DEFAULT_MSG SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR)

if (SDL2_TTF_FOUND)
    set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY})
    set(SDL2_TTF_INCLUDE_DIRS ${SDL2_TTF_INCLUDE_DIR})
endif()
