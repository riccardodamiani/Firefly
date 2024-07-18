#include "entity.h"
#include "gui_panel.h"
#include "gui_element.h"
#include "structures.h"
#include "gui_text.h"
#include "gameEngine.h"

GUI_Panel::GUI_Panel(EntityName objectName, unsigned int elementCode, EntityName textureName, vector2 pos, vector2 rect, int layer) {

	transform.position = pos;
	transform.scale = rect;
	setLayer(layer);

	_texture = new Sprite(layer, textureName);
	this->active = true;
	this->visible = true;
	this->_isMouseOn = false;
	this->_status = false;
	this->_isPressed = false;
	this->elementCode = elementCode;
	
	RegisterObject(objectName);
}

void GUI_Panel::update(double elapsedTime) {
	vector2 mousePos;
	if (this->visible) {
		if (InputEngine::getInstance().wasMouseButtonPressed(SDL_BUTTON_LEFT)) {
			mousePos = GameEngine::getInstance().MousePosition();
			vector2 thisPos = transform.position;
			vector2 thisRect = transform.scale;

			//if mouse left button was clicked inside the button rectangle
			if (mousePos.x >= thisPos.x - thisRect.x / 2.0 &&
				mousePos.x <= thisPos.x + thisRect.x / 2.0 &&
				mousePos.y >= thisPos.y - thisRect.y / 2.0 &&
				mousePos.y <= thisPos.y + thisRect.y / 2.0) {

				GUIEngine::getInstance().RegisterGuiAction(GuiAction::LEFT_BUTTON_DOWN, this);
				return;
			}
		}
	}
	return;
}


bool GUI_Panel::isPanel() {
	return true;
}