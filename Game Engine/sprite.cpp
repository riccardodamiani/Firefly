#include "stdafx.h"
#include "graphics.h"
#include "gameEngine.h"
#include "sprite.h"
#include <mutex>
#include <atomic>

extern Graphics* const _graphicsEngine;

//create a sprite from a existing texture from the name 
Sprite::Sprite(unsigned int screenLayer, EntityName imageName, TextureFlip flip) {
	_imageName = imageName;
	//this->_imageCode = _graphicsEngine->getImageCode(this->_imageName);
	_screenLayer = screenLayer;
	_flip = flip;
}


Sprite::~Sprite() {
	
}

void Sprite::setFlip(TextureFlip flip) {
	std::lock_guard <std::mutex> guard(update_mutex);
	_flip = flip;
}

void Sprite::setTexture(EntityName imageName) {

	std::lock_guard <std::mutex> guard(update_mutex);
	//this->_imageCode = _graphicsEngine->getImageCode(this->_imageName);
	_imageName = imageName;
}

void Sprite::setLayer(uint16_t layer) {
	_screenLayer = layer;
}

uint16_t Sprite::getLayer() {
	return _screenLayer;
}

void Sprite::draw(vector2 pos, vector2 scale, double rot) {

	_graphicsEngine->BlitSurface(_imageName, _screenLayer, pos, scale, rot, _flip);
}

void Sprite::update() {}
