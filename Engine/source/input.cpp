#include "input.h"
#include "graphics.h"
#include "gui.h"

#include <SDL.h>
#include <SDL_events.h>
#include <mutex>
#include <atomic>

InputEngine::InputEngine() {

}


InputEngine::~InputEngine() {

}

void InputEngine::Init(){
	SDL_StopTextInput();
	_didMouseMove = false;
	_lastClickX = 0;
	_lastClickY = 0;
	_lastMousePositionX = 0;
	_lastMousePositionY = 0;
	this->_didMouseWheelMove = false;

	for (Uint8 i = 0; i < _heldMouseKeys.size(); i++) {
		this->_heldMouseKeys[i] = false;
		this->_pressedMouseKeys[i] = false;
		this->_releasedMouseKeys[i] = false;
	}
}

//this function gets called at the start of every frame to reset the keys that are no longer relevant
void InputEngine::beginNewFrame() {

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

	InputEngine::getInstance().getSDLEvent();
	PollRequests();

}

void InputEngine::PollRequests() {

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

void InputEngine::StartTextInput() {
	std::lock_guard <std::mutex> guard(request_mutex);
	_inputPoll.push_back(InputEvent::START_TEXT_INPUT);
}

void InputEngine::StopTextInput() {
	std::lock_guard <std::mutex> guard(request_mutex);
	_inputPoll.push_back(InputEvent::STOP_TEXT_INPUT);
}

InputEvent InputEngine::GetLastEvent() {
	if ((SDL_EventType)_lastEvent.load() == SDL_QUIT) {
		return InputEvent::QUIT_APP;
	}
}


//get a input event and convert it into a sdl event
void InputEngine::getSDLEvent() {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			_lastEvent = (EventType)SDL_QUIT;
			return;
		}		//mouse input
		else if (event.type == SDL_MOUSEBUTTONDOWN) {
			this->mouseKeyDown(&event);
		}
		else if (event.type == SDL_MOUSEBUTTONUP) {
			this->mouseKeyUp(&event);
		}
		else if (event.type == SDL_MOUSEMOTION) {
			this->updateMousePosition(&event);
		}
		else if (event.type == SDL_MOUSEWHEEL) {
			this->_didMouseWheelMove = true;
			_lastWheelMoviment = { event.wheel.x , event.wheel.y };
		}			//text input
		else if (event.type == SDL_TEXTINPUT) {
			GUIEngine::getInstance().appendTextInFocusedEditbox(event.text.text);
		}
		else if (SDL_IsTextInputActive() == SDL_TRUE) {
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_BACKSPACE) {
					GUIEngine::getInstance().backspaceInFocusedEditbox();
				}
				else if (event.key.keysym.sym == SDLK_DELETE) {
					GUIEngine::getInstance().cancelInFocusedEditbox();
				}
				else if (event.key.keysym.sym == SDLK_LEFT) {
					GUIEngine::getInstance().decrementCursorInFocusedEditbox();
				}
				else if (event.key.keysym.sym == SDLK_RIGHT) {
					GUIEngine::getInstance().incrementCursorInFocusedEditbox();
				}

			}
		}		//keyboard input
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.repeat == 0) {
				this->keyDownEvent(&event);
			}
		}
		else if (event.type == SDL_KEYUP) {
			this->keyUpEvent(&event);
		}
	}

	return;
}

//gets called when a key is pressed
void InputEngine::keyDownEvent(const SDL_Event* event) {

	this->_pressedKeys[event->key.keysym.scancode] = true;
	this->_heldKeys[event->key.keysym.scancode] = true;
}

//gets called when a key is released
void InputEngine::keyUpEvent(const SDL_Event* event) {

	this->_releasedKeys[event->key.keysym.scancode] = true;
	this->_heldKeys[event->key.keysym.scancode] = false;
}

//checks if a certain key was pressed during the current frame
bool InputEngine::wasKeyPressed(Scancode key) {
	return this->_pressedKeys[key];
}

//checks if a certain key was released during the current frame
bool InputEngine::wasKeyReleased(Scancode key) {
	return this->_releasedKeys[key];
}

//checks if a certain key is currently being held
bool InputEngine::isKeyHeld(Scancode key) {
	return this->_heldKeys[key];
}

//gets called when a mouse button is pressed
void InputEngine::mouseKeyDown(const SDL_Event* event) {

	this->_pressedMouseKeys[event->button.button] = true;
	this->_heldMouseKeys[event->button.button] = true;
	this->_lastClickX = event->button.x;
	this->_lastClickY = event->button.y;
}

//gets called when a mouse button is released
void InputEngine::mouseKeyUp(const SDL_Event* event) {

	this->_releasedMouseKeys[event->button.button] = true;
	this->_heldMouseKeys[event->button.button] = false;
	this->_lastClickX = event->button.x;
	this->_lastClickY = event->button.y;
}

//checks if a certain mouse button is pressed
bool InputEngine::wasMouseButtonPressed(Uint8 button) {
	return this->_pressedMouseKeys[button];
}

//checks if a certain mouse button is released
bool InputEngine::wasMouseButtonReleased(Uint8 button) {
	return this->_releasedMouseKeys[button];
}

//check if a certain mouse button is being held
bool InputEngine::isMouseButtonHeld(Uint8 button) {
	return this->_heldMouseKeys[button];
}

bool InputEngine::didMouseWheelMove() {
	return this->_didMouseWheelMove;
}


//return the position of the last click of the mouse
std::pair<int, int> InputEngine::getLastClickPosition() {
	std::pair<int, int> click;
	click.first = this->_lastClickX;
	click.second = this->_lastClickY;
	return click;
}


//return the position of the last movement of the mouse
std::pair<int, int> InputEngine::getMousePosition() {
	std::pair<int, int> click;
	click.first = this->_lastMousePositionX;
	click.second = this->_lastMousePositionY;
	return click;
}

std::pair<int, int> InputEngine::getLastWheelMoviment() {
	mouseWheelMove wheel_mov = _lastWheelMoviment.load();
	return std::pair<int, int>(wheel_mov.x, wheel_mov.y);
}

//gets called if the mouse moves
void InputEngine::updateMousePosition(const SDL_Event* event) {

	this->_lastMousePositionX = event->motion.x;
	this->_lastMousePositionY = event->motion.y;
	this->_didMouseMove = true;
}	

//checks if the mouse moved
bool InputEngine::didMouseMove() {
	return this->_didMouseMove;
}




