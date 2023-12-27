#include "stdafx.h"
#include "gui_panel.h"
#include "gui_element.h"
#include "structures.h"
#include "gui_text.h"
#include "gameEngine.h"

extern Graphics* const _graphicsEngine;

GUI_Panel::GUI_Panel(EntityName objectName, unsigned int elementCode, EntityName textureName, vector2 pos, vector2 rect, int layer) {

	transform.position = pos;
	transform.scale = rect;

	_texture = new Sprite(layer, textureName);
	this->active = true;
	this->visible = true;
	this->_isMouseOn = false;
	this->_status = false;
	this->_isPressed = false;
	this->layer = layer;
	this->elementCode = elementCode;
	
	RegisterObject(objectName);
}

void GUI_Panel::update(double elapsedTime) {
	vector2 mousePos;
	if (this->visible) {
		if (_InputEngine->wasMouseButtonPressed(SDL_BUTTON_LEFT)) {
			mousePos = _GameEngine->MousePosition();
			vector2 thisPos = transform.position;
			vector2 thisRect = transform.scale;

			//if mouse left button was clicked inside the button rectangle
			if (mousePos.x >= thisPos.x - thisRect.x / 2.0 &&
				mousePos.x <= thisPos.x + thisRect.x / 2.0 &&
				mousePos.y >= thisPos.y - thisRect.y / 2.0 &&
				mousePos.y <= thisPos.y + thisRect.y / 2.0) {

				_GuiEngine->RegisterGuiAction(GuiAction::LEFT_BUTTON_DOWN, this);
				return;
			}
		}
	}
	return;
}


bool GUI_Panel::isPanel() {
	return true;
}