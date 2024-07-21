# cmake/FindSDL2.cmake

find_path(SDL2_INCLUDE_DIR SDL.h
    PATHS ${CMAKE_SOURCE_DIR}/third-party/SDL/include
)

find_library(SDL2_LIBRARY
    NAMES SDL2
    PATHS ${CMAKE_SOURCE_DIR}/third-party/SDL/lib/x64
)

find_library(SDL2MAIN_LIBRARY
    NAMES SDL2main
    PATHS ${CMAKE_SOURCE_DIR}/third-party/SDL/lib/x64
)

find_path(SDL2_CONFIG_DIR sdl2-config.cmake
    PATHS ${CMAKE_SOURCE_DIR}/third-party/SDL/cmake
)

if (SDL2_CONFIG_DIR)
    set(SDL2_DIR ${SDL2_CONFIG_DIR})
    find_package(SDL2 CONFIG REQUIRED)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2 DEFAULT_MSG SDL2_LIBRARY SDL2_INCLUDE_DIR SDL2MAIN_LIBRARY)

if (SDL2_FOUND)
    set(SDL2_LIBRARIES ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY})
    set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
endif()
