# cmake/FindSDL2_image.cmake

find_path(SDL2_IMAGE_INCLUDE_DIR SDL_image.h
    PATHS ${CMAKE_SOURCE_DIR}/third-party/SDL-image/include
)

find_library(SDL2_IMAGE_LIBRARY
    NAMES SDL2_image
    PATHS ${CMAKE_SOURCE_DIR}/third-party/SDL-image/lib/x64
)

find_path(SDL2_IMAGE_CONFIG_DIR SDL2_imageConfig.cmake
    PATHS ${CMAKE_SOURCE_DIR}/third-party/SDL-image/cmake
)

if (SDL2_IMAGE_CONFIG_DIR)
    set(SDL2_IMAGE_DIR ${SDL2_IMAGE_CONFIG_DIR})
    find_package(SDL2_image CONFIG REQUIRED)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_image DEFAULT_MSG SDL2_IMAGE_LIBRARY SDL2_IMAGE_INCLUDE_DIR)

if (SDL2_IMAGE_FOUND)
    set(SDL2_IMAGE_LIBRARIES ${SDL2_IMAGE_LIBRARY})
    set(SDL2_IMAGE_INCLUDE_DIRS ${SDL2_IMAGE_INCLUDE_DIR})
endif()
