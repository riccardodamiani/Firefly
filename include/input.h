#ifndef INPUT_H
#define INPUT_H

#include <map>
#include <SDL.h>
#include <array>
#include <atomic>
#include <mutex>

struct vector2;

enum class InputEvent {
	START_TEXT_INPUT,
	STOP_TEXT_INPUT,
	CLOSE_WINDOW
};

class InputEngine {
public:
	static InputEngine& getInstance() {
        static InputEngine instance;
        return instance;
    }

    InputEngine(const InputEngine&) = delete;
    InputEngine& operator=(const InputEngine&) = delete;

	Init();

	//set the state of a key or a mouse button
	void beginNewFrame();
	void getSDLEvent();
	void keyUpEvent(const SDL_Event& event);
	void keyDownEvent(const SDL_Event& event);
	void mouseKeyDown(const SDL_Event& event);
	void mouseKeyUp(const SDL_Event& event);
	void updateMousePosition(const SDL_Event& event);

	//returns the state of a key or a mouse button
	bool wasKeyPressed(SDL_Scancode key);
	bool wasKeyReleased(SDL_Scancode key);
	bool isKeyHeld(SDL_Scancode key);
	bool wasMouseButtonPressed(Uint8 button);
	bool wasMouseButtonReleased(Uint8 button);
	bool isMouseButtonHeld(Uint8 button);
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
	InputEngine() = default;
	~InputEngine() = default;

	std::array <std::atomic <bool>, SDL_Scancode::SDL_NUM_SCANCODES> _heldKeys;
	std::array <std::atomic <bool>, SDL_Scancode::SDL_NUM_SCANCODES> _pressedKeys;
	std::array <std::atomic <bool>, SDL_Scancode::SDL_NUM_SCANCODES> _releasedKeys;
	std::array <std::atomic <bool>, 10> _heldMouseKeys;		//256
	std::array <std::atomic <bool>, 10> _pressedMouseKeys;
	std::array <std::atomic <bool>, 10> _releasedMouseKeys;
	std::atomic <int> _lastClickX;
	std::atomic <int> _lastClickY;
	std::atomic <int> _lastMousePositionX;
	std::atomic <int> _lastMousePositionY;
	std::atomic <bool> _didMouseWheelMove;
	std::atomic <std::pair <int, int>> _lastWheelMoviment;
	std::atomic <bool> _didMouseMove;
	std::atomic <SDL_EventType> _lastEvent;
	std::vector <InputEvent> _inputPoll;

	std::mutex request_mutex;
};

#endif
