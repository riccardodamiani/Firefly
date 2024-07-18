#ifndef GAME_OPTIONS_H
#define GAME_OPTIONS_H

#include "graphics.h"

struct GraphicsOptions{
    WindowMode mode;
    uint16_t width, height;
    uint16_t activeLayers;
};

struct AudioOptions{
    std::vector <unsigned short>& groupChannels;
    uint8_t defaultMusicVol;
    uint8_t defaultTrackVol;
};

#endif