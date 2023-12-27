#ifndef SPRITE_H
#define SPRITE_H

#include <string>
#include <atomic>
#include <mutex>
#include "gameEngine.h"
#include "graphics.h"

class Sprite {
public:
	Sprite(unsigned int screenLayer, EntityName imageName = 0, TextureFlip flip = TextureFlip::FLIP_NONE);
	virtual ~Sprite();
	virtual void update();
	void draw(vector2 pos, vector2 scale, double rot);
	void setTexture(EntityName imageName);
	void setFlip(TextureFlip flip = TextureFlip::FLIP_NONE);
private:
	std::atomic <EntityName> _imageName;
	//std::atomic <int> _imageCode;
	std::atomic <int> _screenLayer;
	std::atomic <TextureFlip> _flip;
	std::mutex update_mutex;
};

#endif