#ifndef ANIMATED_SPRITE_H
#define ANIMATED_SPRITE_H

#include "sprite.h"
#include "variables.h"
#include <vector>
#include <atomic>
#include <mutex>

class AnimatedSprite {
public:
	AnimatedSprite(const std::string &imageName, double playTime, unsigned int totalFrames, bool repeat, bool stepByStep, int screenLayer, TextureFlip flip = TextureFlip::FLIP_NONE);
	~AnimatedSprite();
	void update(double elapsedTime);		//update the frame of the animation
	//draws the current frame 
	void draw(vector2 pos, vector2 scale, double rot);
	void setFlip(TextureFlip flip);
	void setFrame(unsigned int frame);
	unsigned int getFramesCount();
	void playAnimation(bool play);		//play the animation
	void nextStep();
	void stopAnimation();		//stops the animation
	bool isPlaying();		//return the playing status
private:
	std::vector <Sprite*> _sprites;
	UInt _currentFrame;
	int _totalFrames;
	Double _elapsedTime;
	Bool _playing;
	Bool _nextStep;
	Bool step_by_step;
	Bool _repeat;

protected:
	double _timeToUpdate;
	void resetAnimation();			//reset the animation
	void animationDone();		//gets called when the animation is done
};

#endif

