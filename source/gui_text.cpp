#include "stdafx.h"
#include "gui_text.h"
#include "gui_element.h"
#include "graphics.h"

extern Graphics* const _graphicsEngine;

//create a text object. Element type must be 'text', and textName must be a unique name
GUI_Text::GUI_Text(EntityName objectName, unsigned int elementCode, EntityName atlasName, vector2 pos, vector2 scale, int layer) {
	
	setLayer(layer);
	transform.position = pos;
	transform.scale = scale;
	this->_text = "";

	this->_atlasName = atlasName;

	_cursorPos = -1;
	_showCursor = false;
	_timeToCursorBlink = 0.5;

	this->_status = false;
	this->_isPressed = false;
	this->elementCode = elementCode;
	this->_minCharNum = 0;
	RegisterObject(objectName);
}

GUI_Text::~GUI_Text() {
	
}


void GUI_Text::update(double elapsedTime) {
	if (!_showCursor)
		return;

	_timeToCursorBlink -= elapsedTime;
	if (_timeToCursorBlink < 0) {
		_timeToCursorBlink = 0.5;
		_cursorBlink = !_cursorBlink;
	}
}

//get the position of the cursor in space
vector2 GUI_Text::getCursorPosition(int cursorIndex) {
	vector2 startScale = transform.scale;
	double startRot = transform.rotation;
	vector2 startPos = transform.position;
	vector2 textSize = _graphicsEngine->GetTextSize(_atlasName, _text, cursorIndex, transform.scale.y());
	vector2 delta = { textSize.x - startScale.x/2, -textSize.y};
	vector2 cursorPos = { startPos.x + delta.x * std::cos(startRot * (M_PI / 180.0)), startPos.y + delta.y + delta.x * std::sin(startRot * (M_PI / 180.0)) };
	return cursorPos;
}

void GUI_Text::draw() {

	if (!this->visible) {
		return;
	}
	_graphicsEngine->BlitTextSurface(_atlasName, _text, getLayer(), transform.position, transform.scale, transform.rotation, TextureFlip::FLIP_NONE, (_cursorBlink && _showCursor) ? _cursorPos : -1);
}

void GUI_Text::ShowCursor(bool show) {
	_cursorBlink = true;
	_showCursor = show;
}

void GUI_Text::SetCursorPos(int pos) {
	_cursorPos = pos;
}

void GUI_Text::setText(std::string text) {
	this->_text = text;
}

std::string GUI_Text::getText() {
	return this->_text;
}

int GUI_Text::getTextLen() {
	return this->_text.size();
}

void GUI_Text::setTextAtlas(EntityName atlasName) {
	_atlasName = atlasName;
}


void GUI_Text::appendText(std::string textToAppend) {
	this->_text += textToAppend;
}

void GUI_Text::deleteChar(int charIndex) {
	this->_text.erase(this->_text.begin() + charIndex);
}

//returns the size of the iserted string
int GUI_Text::insertText(std::string text, int charIndex) {
	if (charIndex >= this->_text.size()) {
		this->_text += text;
	}
	else {
		this->_text.insert(charIndex, text);
	}
	return text.size();
}

bool GUI_Text::isText() {
	return true;
}
