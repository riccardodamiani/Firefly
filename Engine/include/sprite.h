#ifndef SPRITE_H
#define SPRITE_H

#include <string>
#include <atomic>
#include <mutex>

#include "graphics_structs.h"
#include "entity.h"


class Sprite {
public:
	Sprite(unsigned int screenLayer, EntityName imageName = 0, TextureFlip flip = TextureFlip::FLIP_NONE);
	virtual ~Sprite();
	virtual void update();
	void draw(vector2 pos, vector2 scale, double rot);
	void setTexture(EntityName imageName);
	void setFlip(TextureFlip flip = TextureFlip::FLIP_NONE);
	void setLayer(uint16_t layer);
	uint16_t getLayer();
private:
	std::atomic <EntityName> _imageName;
	//std::atomic <int> _imageCode;
	std::atomic <uint16_t> _screenLayer;
	std::atomic <TextureFlip> _flip;
	std::mutex update_mutex;
};

#endif