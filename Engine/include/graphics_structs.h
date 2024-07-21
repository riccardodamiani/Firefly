#ifndef GRAPHICS_STRUCTS_H
#define GRAPHICS_STRUCTS_H
#include "structures.h"

//windows modes
typedef enum class WindowMode{
	MODE_FULLSCREEN = 1,
	MODE_WINDOW_MAX_SIZE = 2,
	MODE_WINDOW = 3
}WindowMode;

typedef enum class TextureFlip
{
	FLIP_NONE = 0x00000000,     /**< Do not flip */
	FLIP_HORIZONTAL = 0x00000001,    /**< flip horizontally */
	FLIP_VERTICAL = 0x00000002     /**< flip vertically */
}TextureFlip;

typedef enum class LightingQuality {
	LOW_QUALITY,	//720p
	MEDIUM_QUALITY,		//1080p
	HIGH_QUALITY		//1440p
}LightingQuality;

struct CustomFilterData {
	int textureWidth, textureHeight;
	int x, y;
	RGBA_Color pixelColor;
	void* args;
};

#endif