#include "entity.h"
#include "gui_element.h"
#include "gui_button.h"
#include "gui.h"
#include "structures.h"
#include "gui_text.h"
#include "gameEngine.h"
#include "sprite.h"

//constructor
GUI_Button::GUI_Button(EntityName objectName, EntityName textureName, unsigned int elementCode, vector2 pos, vector2 rect, int layer) {

	transform.position = pos;
	transform.scale = rect;
	
	setLayer(layer);
	_texture = new Sprite(layer, textureName);
	this->active = true;
	this->visible = true;
	this->_status = false;
	this->_isPressed = false;
	this->_isMouseOn = false;
	this->elementCode = elementCode;

	RegisterObject(objectName);
}

void GUI_Button::update(double elapsedTime) {
	vector2 mousePos;

	if (this->_isMouseOn) {	//mouse was on the button
		vector2 thisPos = transform.position;
		vector2 thisRect = transform.scale;
		mousePos = GameEngine::getInstance().MousePosition();
		if ( !( mousePos.x >= thisPos.x - thisRect.x / 2.0 &&	//out of the rectangle
				mousePos.x <= thisPos.x + thisRect.x / 2.0 &&
				mousePos.y >= thisPos.y - thisRect.y / 2.0 &&
				mousePos.y <= thisPos.y + thisRect.y / 2.0 )  ) {
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

	//you can get down here only if the mouse is on the button so no need to recheck it
	if (this->_isPressed) {		//button was pressed
		if (InputEngine::getInstance().wasMouseButtonReleased(SDL_BUTTON_LEFT)) {
			this->_isPressed = false;
			GUIEngine::getInstance().RegisterGuiAction(GuiAction::LEFT_BUTTON_UP, this);
			return;
		}
	}
	else {		//button is not pressed
		if (InputEngine::getInstance().wasMouseButtonPressed(SDL_BUTTON_LEFT)) {
			GUIEngine::getInstance().RegisterGuiAction(GuiAction::LEFT_BUTTON_DOWN, this);
			return;
		}
	}
}

//negate the status of the element and return the new status
bool GUI_Button::negateStatus() {
	this->_status = !this->_status;
	return this->_status;
}

bool GUI_Button::isButton() {
	return true;
}