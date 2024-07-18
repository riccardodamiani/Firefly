#include "entity.h"
#include "gui_element.h"
#include "gui_slider.h"
#include "structures.h"
#include "gui_text.h"
#include "AnimatedSprite.h"
#include "gameEngine.h"


//constructor
GUI_Slider::GUI_Slider(EntityName objectName, unsigned int elementCode, std::vector <AnimatedSprite*> &animatedSprite, vector2 pos, vector2 rect,
	double minSliderVal, double maxSliderVal, double minResponseRange, double maxResponseRate, int layer){

	setLayer(layer);
	_spriteAnimations = animatedSprite;

	AnimateSprite(true);
	SetActiveSpriteAnimation(0);

	transform.position = pos;
	transform.scale = rect;

	this->active = true;
	this->visible = true;
	this->_status = false;
	this->_isPressed = false;
	this->_isMouseOn = false;
	this->elementCode = elementCode;
	wasChanged = true;

	this->_minValue = minSliderVal;
	this->_maxValue = maxSliderVal;

	this->_minRange = minResponseRange;
	this->_maxRange = maxResponseRate;

	RegisterObject(objectName);
}

GUI_Slider::GUI_Slider(EntityName objectName, unsigned int elementCode, AnimatedSprite* spriteAnim, vector2 pos, vector2 rect,
	double minSliderVal, double maxSliderVal, double minResponseRange,
	double maxResponseRate, int layer) {

	setLayer(layer);
	if(spriteAnim != nullptr)
		_spriteAnimations.push_back(spriteAnim);

	AnimateSprite(true);
	SetActiveSpriteAnimation(0);

	transform.position = pos;
	transform.scale = rect;

	this->active = true;
	this->visible = true;
	this->_status = false;
	this->_isPressed = false;
	this->_isMouseOn = false;
	this->elementCode = elementCode;
	wasChanged = true;

	this->_minValue = minSliderVal;
	this->_maxValue = maxSliderVal;

	this->_minRange = minResponseRange;
	this->_maxRange = maxResponseRate;

	RegisterObject(objectName);
}


bool GUI_Slider::ValueChanged() {
	bool temp = wasChanged;
	wasChanged = false;
	return temp;
}

void GUI_Slider::SetValue(double value) {

	std::lock_guard <std::mutex> guard(update_mutex);
	if (value > _maxValue)
		value = _maxValue;
	if (value < _minValue)
		value = _minValue;
	_value = value;
	wasChanged = true;
	if (spriteAnimationID < _spriteAnimations.size()) {
		int frames = this->_spriteAnimations[spriteAnimationID]->getFramesCount();

		this->_spriteAnimations[spriteAnimationID]->setFrame((int)(((this->_value - this->_minValue) / (this->_maxValue - this->_minValue)) * (frames - 1)));
	}
}

void GUI_Slider::updateAnimation() {
	SetValue(_value);
}

double GUI_Slider::getSliderValue() {
	return this->_value;
}


void GUI_Slider::update(double elapsedTime) {
	vector2 mousePos;

	if (this->_isMouseOn) {	//mouse was on the button
		vector2 thisPos = transform.position;
		vector2 thisRect = transform.scale;
		mousePos = GameEngine::getInstance().MousePosition();
		if (!(mousePos.x >= thisPos.x - thisRect.x / 2.0 &&	//out of the rectangle
			mousePos.x <= thisPos.x + thisRect.x / 2.0 &&
			mousePos.y >= thisPos.y - thisRect.y / 2.0 &&
			mousePos.y <= thisPos.y + thisRect.y / 2.0)) {
			GUIEngine::getInstance().RegisterGuiAction(GuiAction::MOUSE_MOVED_OUT, this);
			this->_isMouseOn = false;
			this->_isPressed = false;
			return;
		}
		else {
			GUIEngine::getInstance().RegisterGuiAction(GuiAction::MOUSE_HOVERING, this);
		}
	}
	else {
		vector2 thisPos = transform.position;
		vector2 thisRect = transform.scale;
		mousePos = GameEngine::getInstance().MousePosition();
		if (mousePos.x >= thisPos.x - thisRect.x / 2.0 &&	//mouse on
			mousePos.x <= thisPos.x + thisRect.x / 2.0 &&
			mousePos.y >= thisPos.y - thisRect.y / 2.0 &&
			mousePos.y <= thisPos.y + thisRect.y / 2.0) {

			GUIEngine::getInstance().RegisterGuiAction(GuiAction::MOUSE_MOVED_OVER, this);
			return;
		}
		return;
	}

	//you can get down here only if the mouse is on the slider so no need to recheck it
	if (InputEngine::getInstance().isMouseButtonHeld(SDL_BUTTON_LEFT)) {
		vector2 thisPos = transform.position;
		vector2 thisRect = transform.scale;
		mousePos = GameEngine::getInstance().MousePosition();
		//leaves the gui engine decide if the slider was pressed or not
		_lastMousePosition = mousePos;
		GUIEngine::getInstance().RegisterGuiAction(GuiAction::LEFT_BUTTON_DOWN, this);
	}
}

void GUI_Slider::applyAction(GuiAction mouseAction) {
	switch (mouseAction) {
	case GuiAction::LEFT_BUTTON_DOWN:	//set the slider value
	{
		vector2 thisPos = transform.position;
		vector2 thisRect = transform.scale;
		double sliderSize = thisRect.x * (_maxRange - _minRange);
		double sliderStart = (thisPos.x - thisRect.x/2) + thisRect.x * _minRange;
		double sliderEnd = sliderStart + sliderSize;
		double newVal = this->_minValue + (_lastMousePosition.x - sliderStart) * ((_maxValue - _minValue) / (sliderEnd - sliderStart));
		SetValue(newVal);
		break;
	}

	case GuiAction::MOUSE_MOVED_OVER:	//do not add breaks here
	case GuiAction::MOUSE_HOVERING:
		this->_isMouseOn = true;
		break;
	case GuiAction::MOUSE_MOVED_OUT:
		this->_isMouseOn = false;
		this->_isPressed = false;
		break;
	}
}

bool GUI_Slider::isSlider() {
	return true;
}