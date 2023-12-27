#include "stdafx.h"
#include "graphics.h"
#include "gameEngine.h"
#include "sprite.h"
#include <mutex>
#include <atomic>

extern Graphics* const _graphicsEngine;

//create a sprite from a existing texture from the name 
Sprite::Sprite(unsigned int screenLayer, EntityName imageName, TextureFlip flip) {
	this->_imageName = imageName;
	//this->_imageCode = _graphicsEngine->getImageCode(this->_imageName);
	this->_screenLayer = screenLayer;
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
	this->_imageName = imageName;
}

void Sprite::draw(vector2 pos, vector2 scale, double rot) {

	_graphicsEngine->BlitSurface(_imageName, this->_screenLayer, pos, scale, rot, _flip);
}

void Sprite::update() {}
