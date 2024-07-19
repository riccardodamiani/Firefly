#ifndef GAME_OPTIONS_H
#define GAME_OPTIONS_H

#include <vector>
#include <stdint.h>

#include "graphics_structs.h"

struct GraphicsOptions{
    WindowMode mode;
    uint16_t width, height;
    uint16_t activeLayers;
};

struct AudioOptions{
    std::vector <unsigned short> groupChannels;
    uint8_t defaultMusicVol;
    uint8_t defaultTrackVol;
};

#endif