#include "stdafx.h"
#include "GUI.h"
#include "input.h"
#include "gui_element.h"
#include "gui_button.h"
#include "gui_panel.h"
#include "gui_text.h"
#include "gui_slider.h"
#include "gui_editbox.h"
#include "graphics.h"
#include "structures.h"
#include "game.h"

#include <map>
#include <mutex>

extern Graphics* const _graphicsEngine;

GUI::GUI() {
	
}


void GUI::RegisterGuiAction(GuiAction action, GUI_Element* element) {
	std::lock_guard <std::mutex> guard(update_mutex);
	elementActions.push_back({ element , action });
}

void GUI::appendTextInFocusedEditbox(std::string text) {

	if (text == "")
		return;

	GUI_Element* f = focusedElement;
	if (f != nullptr && f->isEditbox()) {
		((GUI_Editbox*)f)->insertText(text);
	}
}

void GUI::incrementCursorInFocusedEditbox() {
	GUI_Element* f = focusedElement;
	if (f != nullptr && f->isEditbox()) {
		((GUI_Editbox*)f)->incrementCursorPos();
	}
}

void GUI::decrementCursorInFocusedEditbox() {
	GUI_Element* f = focusedElement;
	if (f != nullptr && f->isEditbox()) {
		((GUI_Editbox*)f)->decrementCursorPos();
	}
}

void GUI::backspaceInFocusedEditbox() {
	GUI_Element* f = focusedElement;
	if (f != nullptr && f->isEditbox()) {
		((GUI_Editbox*)f)->backspace();
	}
}

void GUI::cancelInFocusedEditbox() {
	GUI_Element* f = focusedElement;
	if (f != nullptr && f->isEditbox()) {
		((GUI_Editbox*)f)->cancel();
	}
}

void GUI::clearFocus() {
	std::lock_guard <std::mutex> guard(focus_related_mutex);
	focusedElement = nullptr;
	_InputEngine->StopTextInput();
}

void GUI::beginNewFrame() {

	std::lock_guard <std::mutex> guard(update_mutex);
	if (elementActions.size() == 0)
		return;

	for (int i = 0; i < elementActions.size(); i++) {		//handle easy actions
		if (elementActions[i].action == GuiAction::REMOVE_FOCUS) {
			_GameEngine->_GuiListener(elementActions[i].element, elementActions[i].action);
			elementActions[i].element->applyAction(GuiAction::REMOVE_FOCUS);
			std::lock_guard <std::mutex> guard(focus_related_mutex);
			focusedElement = nullptr;
			elementActions.erase(elementActions.begin() + i);
			--i;
		}
		else if (elementActions[i].action == GuiAction::MOUSE_MOVED_OUT) {
			_GameEngine->_GuiListener(elementActions[i].element, elementActions[i].action);
			elementActions.erase(elementActions.begin() + i);
			--i;
		}
	}

	//handle mutually exclusive actions
	int maxLayer = -1;
	for (int i = 0; i < elementActions.size(); i++) {	//find the highest layer
		int layer = elementActions[i].element->layer;
		if (layer > maxLayer) {
			maxLayer = layer;
		}
	}
	for (int i = 0; i < elementActions.size(); i++) {
		if (elementActions[i].element->layer == maxLayer) {		//action of the top element is approved
			if (elementActions[i].action == GuiAction::FOCUS) {		//requested focus
				std::lock_guard <std::mutex> guard(focus_related_mutex);
				focusedElement = elementActions[i].element;
			}
			elementActions[i].element->applyAction(elementActions[i].action);
			_GameEngine->_GuiListener(elementActions[i].element, elementActions[i].action);
			
		}
		else {		//all other actions are denied
			if (elementActions[i].action != GuiAction::MOUSE_MOVED_OVER) {
				elementActions[i].element->applyAction(GuiAction::MOUSE_MOVED_OUT);
				_GameEngine->_GuiListener(elementActions[i].element, GuiAction::MOUSE_MOVED_OUT);
			}
		}
	}
	elementActions.clear();
}

