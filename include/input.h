#ifndef INPUT_H
#define INPUT_H

#include <map>
#include <array>
#include <atomic>
#include <mutex>
#include <vector>

#include "scancode.h"
#include "gameEvent.h"
#include "engine_exports.h"

struct vector2;
union SDL_Event;

enum class InputEvent {
	START_TEXT_INPUT,
	STOP_TEXT_INPUT,
	QUIT_APP
};

class ENGINE_API InputEngine {
public:
	static InputEngine& getInstance() {
        static InputEngine instance;
        return instance;
    }

    InputEngine(const InputEngine&) = delete;
    InputEngine& operator=(const InputEngine&) = delete;

	void Init();

	//set the state of a key or a mouse button
	void beginNewFrame();
	void getSDLEvent();
	void keyUpEvent(const SDL_Event* event);
	void keyDownEvent(const SDL_Event* event);
	void mouseKeyDown(const SDL_Event* event);
	void mouseKeyUp(const SDL_Event* event);
	void updateMousePosition(const SDL_Event* event);

	//returns the state of a key or a mouse button
	bool wasKeyPressed(Scancode key);
	bool wasKeyReleased(Scancode key);
	bool isKeyHeld(Scancode key);
	bool wasMouseButtonPressed(uint8_t button);
	bool wasMouseButtonReleased(uint8_t button);
	bool isMouseButtonHeld(uint8_t button);
	bool didMouseWheelMove();
	bool didMouseMove();

	std::pair<int, int> getLastWheelMoviment();
	std::pair<int, int> getLastClickPosition();
	std::pair<int, int> getMousePosition();

	void PollRequests();
	void StartTextInput();
	void StopTextInput();
	InputEvent GetLastEvent();

private:
	InputEngine();
	~InputEngine();

	std::array <std::atomic <bool>, 512> _heldKeys; //SDL_Scancode::SDL_NUM_SCANCODES
	std::array <std::atomic <bool>, 512> _pressedKeys; //SDL_Scancode::SDL_NUM_SCANCODES
	std::array <std::atomic <bool>, 512> _releasedKeys; //SDL_Scancode::SDL_NUM_SCANCODES
	std::array <std::atomic <bool>, 10> _heldMouseKeys;
	std::array <std::atomic <bool>, 10> _pressedMouseKeys;
	std::array <std::atomic <bool>, 10> _releasedMouseKeys;
	std::atomic <int> _lastClickX;
	std::atomic <int> _lastClickY;
	std::atomic <int> _lastMousePositionX;
	std::atomic <int> _lastMousePositionY;
	std::atomic <bool> _didMouseWheelMove;
	std::atomic <std::pair <int, int>> _lastWheelMoviment;
	std::atomic <bool> _didMouseMove;
	std::atomic <EventType> _lastEvent;
	std::vector <InputEvent> _inputPoll;

	std::mutex request_mutex;
};

#endif
