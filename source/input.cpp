#include "stdafx.h"
#include "input.h"
#include "graphics.h"
#include "gui.h"
#include <mutex>
#include <atomic>

extern Graphics* const _graphicsEngine;

Input::Input() {
	this->_didMouseWheelMove = false;
	_didMouseMove = false;
	_gui = nullptr;
	_lastClickX = 0;
	_lastClickY = 0;
	_lastMousePositionX = 0;
	_lastMousePositionY = 0;
	for (Uint8 i = 0; i < _heldMouseKeys.size(); i++) {
		this->_heldMouseKeys[i] = false;
		this->_pressedMouseKeys[i] = false;
		this->_releasedMouseKeys[i] = false;
	}
}

Input::Input(GUI& gui) {

	SDL_StopTextInput();
	_didMouseMove = false;
	_lastClickX = 0;
	_lastClickY = 0;
	_lastMousePositionX = 0;
	_lastMousePositionY = 0;
	this->_gui = &gui;
	this->_didMouseWheelMove = false;

	for (Uint8 i = 0; i < _heldMouseKeys.size(); i++) {
		this->_heldMouseKeys[i] = false;
		this->_pressedMouseKeys[i] = false;
		this->_releasedMouseKeys[i] = false;
	}
}

//this function gets called at the start of every frame to reset the keys that are no longer relevant
void Input::beginNewFrame() {

	for (int i = 0; i < _pressedKeys.size(); i++) {
		_pressedKeys[i] = 0;
		_releasedKeys[i] = 0;
	}
	for (int i = 0; i < _pressedMouseKeys.size(); i++) {
		_pressedMouseKeys[i] = 0;
		_releasedMouseKeys[i] = 0;
	}

	this->_didMouseWheelMove = false;
	this->_didMouseMove = false;

	_InputEngine->getSDLEvent();
	PollRequests();

}

void Input::PollRequests() {

	std::lock_guard <std::mutex> guard(request_mutex);

	for (int i = 0; i < _inputPoll.size(); i++) {
		if (_inputPoll[i] == InputEvent::START_TEXT_INPUT) {
			SDL_StartTextInput();
		}
		else if (_inputPoll[i] == InputEvent::STOP_TEXT_INPUT) {
			SDL_StopTextInput();
		}
	}
	_inputPoll.clear();
}

void Input::StartTextInput() {
	std::lock_guard <std::mutex> guard(request_mutex);
	_inputPoll.push_back(InputEvent::START_TEXT_INPUT);
}

void Input::StopTextInput() {
	std::lock_guard <std::mutex> guard(request_mutex);
	_inputPoll.push_back(InputEvent::STOP_TEXT_INPUT);
}

InputEvent Input::GetLastEvent() {
	if (_lastEvent == SDL_QUIT) {
		return InputEvent::CLOSE_WINDOW;
	}
}


//get a input event and convert it into a sdl event
void Input::getSDLEvent() {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			_lastEvent = SDL_QUIT;
			return;
		}		//mouse input
		else if (event.type == SDL_MOUSEBUTTONDOWN) {
			this->mouseKeyDown(event);
		}
		else if (event.type == SDL_MOUSEBUTTONUP) {
			this->mouseKeyUp(event);
		}
		else if (event.type == SDL_MOUSEMOTION) {
			this->updateMousePosition(event);
		}
		else if (event.type == SDL_MOUSEWHEEL) {
			this->_didMouseWheelMove = true;
			_lastWheelMoviment = { event.wheel.x , event.wheel.y };
		}			//text input
		else if (event.type == SDL_TEXTINPUT) {
			this->_gui->appendTextInFocusedEditbox(event.text.text);
		}
		else if (SDL_IsTextInputActive() == SDL_TRUE) {
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_BACKSPACE) {
					this->_gui->backspaceInFocusedEditbox();
				}
				else if (event.key.keysym.sym == SDLK_DELETE) {
					this->_gui->cancelInFocusedEditbox();
				}
				else if (event.key.keysym.sym == SDLK_LEFT) {
					this->_gui->decrementCursorInFocusedEditbox();
				}
				else if (event.key.keysym.sym == SDLK_RIGHT) {
					this->_gui->incrementCursorInFocusedEditbox();
				}

			}
		}		//keyboard input
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.repeat == 0) {
				this->keyDownEvent(event);
			}
		}
		else if (event.type == SDL_KEYUP) {
			this->keyUpEvent(event);
		}
	}

	return;
}

//gets called when a key is pressed
void Input::keyDownEvent(const SDL_Event& event) {

	this->_pressedKeys[event.key.keysym.scancode] = true;
	this->_heldKeys[event.key.keysym.scancode] = true;
}

//gets called when a key is released
void Input::keyUpEvent(const SDL_Event& event) {

	this->_releasedKeys[event.key.keysym.scancode] = true;
	this->_heldKeys[event.key.keysym.scancode] = false;
}

//checks if a certain key was pressed during the current frame
bool Input::wasKeyPressed(SDL_Scancode key) {
	return this->_pressedKeys[key];
}

//checks if a certain key was released during the current frame
bool Input::wasKeyReleased(SDL_Scancode key) {
	return this->_releasedKeys[key];
}

//checks if a certain key is currently being held
bool Input::isKeyHeld(SDL_Scancode key) {
	return this->_heldKeys[key];
}

//gets called when a mouse button is pressed
void Input::mouseKeyDown(const SDL_Event& event) {

	this->_pressedMouseKeys[event.button.button] = true;
	this->_heldMouseKeys[event.button.button] = true;
	this->_lastClickX = event.button.x;
	this->_lastClickY = event.button.y;
}

//gets called when a mouse button is released
void Input::mouseKeyUp(const SDL_Event& event) {

	this->_releasedMouseKeys[event.button.button] = true;
	this->_heldMouseKeys[event.button.button] = false;
	this->_lastClickX = event.button.x;
	this->_lastClickY = event.button.y;
}

//checks if a certain mouse button is pressed
bool Input::wasMouseButtonPressed(Uint8 button) {
	return this->_pressedMouseKeys[button];
}

//checks if a certain mouse button is released
bool Input::wasMouseButtonReleased(Uint8 button) {
	return this->_releasedMouseKeys[button];
}

//check if a certain mouse button is being held
bool Input::isMouseButtonHeld(Uint8 button) {
	return this->_heldMouseKeys[button];
}

bool Input::didMouseWheelMove() {
	return this->_didMouseWheelMove;
}


//return the position of the last click of the mouse
std::pair<int, int> Input::getLastClickPosition() {
	std::pair<int, int> click;
	click.first = this->_lastClickX;
	click.second = this->_lastClickY;
	return click;
}


//return the position of the last movement of the mouse
std::pair<int, int> Input::getMousePosition() {
	std::pair<int, int> click;
	click.first = this->_lastMousePositionX;
	click.second = this->_lastMousePositionY;
	return click;
}

std::pair<int, int> Input::getLastWheelMoviment() {
	return this->_lastWheelMoviment;
}

//gets called if the mouse moves
void Input::updateMousePosition(const SDL_Event& event) {

	this->_lastMousePositionX = event.motion.x;
	this->_lastMousePositionY = event.motion.y;
	this->_didMouseMove = true;
}	

//checks if the mouse moved
bool Input::didMouseMove() {
	return this->_didMouseMove;
}




