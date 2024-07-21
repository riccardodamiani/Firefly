#ifndef GUI_H
#define GUI_H

#include <map>
#include <mutex>
#include <vector>
#include <atomic>

#include "engine_exports.h"

class Input;
class GUI_Element;

//mouse action codes for gui actions
enum class GuiAction {
	UNKNOWN,
	LEFT_BUTTON_DOWN,
	LEFT_BUTTON_UP,
	RIGHT_BUTTON_DOWN,
	RIGHT_BUTTON_UP,
	MIDDLE_BUTTON_DOWN,
	MIDDLE_BUTTON_UP,
	MOUSE_HOVERING,
	MOUSE_MOVED_OVER,
	MOUSE_MOVED_OUT,
	REMOVE_FOCUS,
	FOCUS
};

typedef struct action {
	GUI_Element* element;
	GuiAction action;
}Action;

class ENGINE_API GUIEngine {
public:
	static GUIEngine& getInstance() {
        static GUIEngine instance;
        return instance;
    }

    GUIEngine(const GUIEngine&) = delete;
    GUIEngine& operator=(const GUIEngine&) = delete;

	void incrementCursorInFocusedEditbox();
	void decrementCursorInFocusedEditbox();
	void backspaceInFocusedEditbox();
	void cancelInFocusedEditbox();
	void appendTextInFocusedEditbox(std::string text);

	void RegisterGuiAction(GuiAction action, GUI_Element *element);

	void beginNewFrame();
	void clearFocus();
private:
	GUIEngine();
	~GUIEngine();
	std::atomic <GUI_Element*> focusedElement;
	std::vector <Action> elementActions;
	std::mutex update_mutex;
	std::mutex focus_related_mutex;
};

#endif