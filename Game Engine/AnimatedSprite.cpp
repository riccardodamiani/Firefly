#include "stdafx.h"
#include "AnimatedSprite.h"
#include "graphics.h"

extern Graphics* const _graphicsEngine;

AnimatedSprite::~AnimatedSprite() {
	for (int i = 0; i < this->_sprites.size(); i++) {
		delete this->_sprites[i];
	}
	this->_sprites.clear();
}

AnimatedSprite::AnimatedSprite(const std::string &imageName, double playTime, unsigned int totalFrames, bool repeat, bool stepByStep, int screenLayer, TextureFlip flip) {

	if (totalFrames == 0) {
		return;
	}
	this->_timeToUpdate = playTime / totalFrames;
	this->_totalFrames = totalFrames;
	for (int i = 0; i < this->_totalFrames; i++) {
		std::string spriteName = imageName + "_" + std::to_string(i);
		this->_sprites.push_back(new Sprite(screenLayer, DecodeName(spriteName.c_str()), flip));
	}

	step_by_step = stepByStep;
	this->_currentFrame = 0;
	this->_elapsedTime = 0.0;
	this->_playing = false;
	this->_repeat = repeat;
}


void AnimatedSprite::setFrame(unsigned int frame) {

	if(frame < this->_sprites.size())
	{
		this->_currentFrame = frame;
	}
}

unsigned int AnimatedSprite::getFramesCount() {
	return this->_sprites.size();
}

//update the frame of the animation
void AnimatedSprite::update(double elapsedTime) {
	
	if (this->_playing) {
		if (step_by_step && !_nextStep) {
			resetAnimation();
			return;
		}

		if ((step_by_step && _nextStep) || !step_by_step) {
			_nextStep = false;
			this->_elapsedTime += elapsedTime;
			if (this->_elapsedTime >= this->_timeToUpdate) {
				this->_elapsedTime -= this->_timeToUpdate;
				if (this->_currentFrame < this->_totalFrames - 1) {
					this->_currentFrame += 1;
				}
				else {
					this->animationDone();
				}
			}
		}
	}
}

//reset the animation
void AnimatedSprite::resetAnimation() {
	this->_currentFrame = 0;
	this->_elapsedTime = 0;
}

//play the animation
void AnimatedSprite::playAnimation(bool play) {
	this->_playing = play;
}

//stop the animation
void AnimatedSprite::stopAnimation() {
	this->_playing = false;
	this->_currentFrame = 0;
	this->_elapsedTime = 0;
}

void AnimatedSprite::nextStep() {
	_nextStep = true;
}

//gets called when an animation finish
void AnimatedSprite::animationDone() {
	if (this->_repeat) {
		this->stopAnimation();
		this->playAnimation(true);
	}
	else {
		this->stopAnimation();
	}
}

void AnimatedSprite::draw(vector2 pos, vector2 anim_scale, double rot) {
	Sprite *sprite = this->_sprites[this->_currentFrame];
	sprite->draw(pos, anim_scale, rot);
}

void AnimatedSprite::setFlip(TextureFlip flip) {

	for (int i = 0; i < this->_sprites.size(); i++) {
		_sprites[i]->setFlip(flip);
	}
}

bool AnimatedSprite::isPlaying() {
	return this->_playing;
}

