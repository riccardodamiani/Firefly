
#include "entity.h"
#include "gui_editbox.h"
#include "gui_text.h"
#include "graphics.h"
#include "gameEngine.h"
#include "sprite.h"

#include <SDL.h>

GUI_Editbox::GUI_Editbox(EntityName objName, unsigned int elementCode, EntityName textureName, std::string hintText, EntityName textFontAlias, EntityName hintFontAlias,
	vector2 pos, vector2 scale, int layer) {
	
	setLayer(layer);
	transform.position = pos;
	transform.scale = scale;
	_texture = new Sprite(layer, textureName);

	this->active = true;
	this->visible = true;
	this->_status = false;
	this->_isPressed = false;
	this->_isMouseOn = false;
	this->elementCode = elementCode;

	this->_isNumerical = false;
	this->_cursor = 0;

	vector2 textScale = {scale.x*0.9, scale.y*0.9};
	RegisterObject(objName);

	this->_text = new GUI_Text(0, -1, textFontAlias, pos, textScale, getLayer() + 1);
	_text->SetVisible(true);
	_text->SetCursorPos(0);
	_text->SetConstraintParent(this, true, true, true);
	this->_hintText = new GUI_Text(0, -1, hintFontAlias, pos, textScale, getLayer() + 1);
	this->_hintText->setText(hintText);
	_hintText->SetConstraintParent(this, true, true, true);
	
}

GUI_Editbox::~GUI_Editbox() {
	delete _text;
	delete this->_hintText;
}

void GUI_Editbox::applyAction(GuiAction mouseAction) {

	switch (mouseAction) {
	case GuiAction::FOCUS:
		this->_isPressed = false;
		InputEngine::getInstance().StartTextInput();
		_text->ShowCursor(true);
		this->_status = true;
		break;
	case GuiAction::REMOVE_FOCUS:
		this->_status = false;
		InputEngine::getInstance().StopTextInput();
		_text->ShowCursor(false);
		break;

	case GuiAction::LEFT_BUTTON_DOWN:
		this->_isPressed = true;
		break;

	case GuiAction::MOUSE_MOVED_OVER:		//here there shouldn't be a break
	case GuiAction::MOUSE_HOVERING:
		this->_isMouseOn = true;
		break;

	case GuiAction::MOUSE_MOVED_OUT:
		this->_isMouseOn = false;
		this->_isPressed = false;
		break;
	}
}

void GUI_Editbox::update(double elapsedTime) {
	vector2 mousePos;

	if (_status && InputEngine::getInstance().wasMouseButtonPressed(SDL_BUTTON_LEFT)) {	//remove focus
		vector2 thisPos = transform.position;
		vector2 thisRect = transform.scale;
		mousePos = GameEngine::getInstance().MousePosition();
		if (!(mousePos.x >= thisPos.x - thisRect.x / 2.0 &&	//out of the rectangle
			mousePos.x <= thisPos.x + thisRect.x / 2.0 &&
			mousePos.y >= thisPos.y - thisRect.y / 2.0 &&
			mousePos.y <= thisPos.y + thisRect.y / 2.0)) {
			GUIEngine::getInstance().RegisterGuiAction(GuiAction::REMOVE_FOCUS, this);
			this->_isMouseOn = false;
			this->_isPressed = false;
			return;
		}
	}

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

	//you can get down here only if the mouse is on the button so no need to recheck it
	if (this->_isPressed) {		//button was pressed
		if (InputEngine::getInstance().wasMouseButtonReleased(SDL_BUTTON_LEFT)) {
			this->_isPressed = false;
			GUIEngine::getInstance().RegisterGuiAction(GuiAction::FOCUS, this);
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


void clear() {

}

void GUI_Editbox::setActive(bool active) {
	
	std::lock_guard <std::mutex> guard(update_mutex);
	this->active = active;
	this->_status = false;
	this->_isPressed = false;
	this->_isMouseOn = false;

	if (!this->active && _status) {		//if is focused
		InputEngine::getInstance().StopTextInput();
		GUIEngine::getInstance().RegisterGuiAction(GuiAction::REMOVE_FOCUS, this);
	}
}


std::string GUI_Editbox::getText() {
	return this->_text->getText();
}


void GUI_Editbox::setText(std::string text) {

	this->_text->setText(text);
	if(text == "")
		_hintText->SetVisible(true);
	else _hintText->SetVisible(false);
}

void GUI_Editbox::setHintText(std::string hintText) {
	this->_hintText->setText(hintText);
}

//true accept only numbers as input
void GUI_Editbox::DigitOnly(bool digitOnly) {
	
	std::lock_guard <std::mutex> guard(text_related_mutex);
	this->_isNumerical = digitOnly;
}

void GUI_Editbox::backspace() {
	std::lock_guard <std::mutex> guard(text_related_mutex);
	if (this->_cursor > 0) {

		this->_text->deleteChar(this->_cursor - 1);
		this->_cursor-=1;
		_text->SetCursorPos(_cursor);
	}
	if (_text->getTextLen() == 0) {
		_hintText->SetVisible(true);
	}
}

void GUI_Editbox::cancel() {
	std::lock_guard <std::mutex> guard(text_related_mutex);
	if (this->_cursor < this->_text->getTextLen()) {
		this->_text->deleteChar(this->_cursor);
		_text->SetCursorPos(_cursor);
	}
	if (_text->getTextLen() == 0) {
		_hintText->SetVisible(true);
	}
}

void GUI_Editbox::incrementCursorPos() {
	std::lock_guard <std::mutex> guard(text_related_mutex);
	if (this->_cursor < this->_text->getTextLen()) {
		this->_cursor+=1;
		_text->SetCursorPos(_cursor);
	}
}

void GUI_Editbox::decrementCursorPos() {
	std::lock_guard <std::mutex> guard(text_related_mutex);
	if (this->_cursor > 0) {
		this->_cursor-=1;
		_text->SetCursorPos(_cursor);
	}
}

void GUI_Editbox::insertText(std::string text) {
	std::lock_guard <std::mutex> guard(text_related_mutex);
	if (this->_isNumerical) {
		std::string::const_iterator it = text.begin();
		while (it != text.end()) {
			if (!std::isdigit(*it)) {	//not a number so it gets erased
				text.erase(it);
				continue;
			}
			++it;
		}
	}
	this->_cursor += this->_text->insertText(text, this->_cursor);
	_text->SetCursorPos(_cursor);

	if (_text->getTextLen() > 0) {
		_hintText->SetVisible(false);
	}
}

bool GUI_Editbox::isEditbox() {
	return true;
}
